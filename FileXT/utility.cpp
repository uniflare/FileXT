#include "pch.h"
#include "utility.h"

#include "common.h"
#include "platform.h"

#if defined(WIN32)
#include <shellapi.h>
#include <shlobj.h>
#include <direct.h>
#ifndef getcwd
#define getcwd _getcwd
#endif
#endif

#ifdef __linux__
#include <dlfcn.h>
#include <unistd.h>
#include <fstream>
#endif

namespace filext
{
	const std::filesystem::path& tryGetDllFolder() {
		static std::filesystem::path dllFolder;

		if (dllFolder.empty()) {
#ifdef _MSC_VER
			// Borrowed from https://gist.github.com/pwm1234/05280cf2e462853e183d

			char path[1024];
			HMODULE hm = NULL;

			if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
				GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
				(LPCSTR)tryGetDllFolder,
				&hm)) {
				LOG("error: GetModuleHandle returned %i", GetLastError());
				dllFolder.clear();
			}
			else {
				GetModuleFileNameA(hm, path, sizeof(path));
				dllFolder = path;
			}
#else 
			Dl_info dl_info;
			dladdr((void*)tryGetDllFolder, &dl_info);
			dllFolder = dl_info.dli_fname;
#endif
			if (!dllFolder.empty()) {
				dllFolder = dllFolder.parent_path();
			}
		}

		return dllFolder;
	}

	std::filesystem::path tryGetStorageFolder() {
		std::filesystem::path path;

		const std::vector<std::string> args = process::tryGetCommandLineArgs();
		std::string profileName;
		std::string profileFolder;

		for (auto& arg : args) {
			if (string::startsWith(arg, "-name=")) {
				profileName = arg.substr(6);
			}
			else if (string::startsWith(arg, "-profiles=")) {
				profileFolder = arg.substr(10);
			}

			if (!profileName.empty() && !profileFolder.empty()) {
				break;
			}
		}

#if defined(WIN32)
		// Folder = Documents/Arma 3 (Default)
		if (profileFolder.empty()) {
			PWSTR documentPath = nullptr;
			HRESULT hresult = SHGetKnownFolderPath(FOLDERID_Documents, NULL, NULL, &documentPath);
			if (SUCCEEDED(hresult)) {
				std::string temp;
				size_t len = wcstombs(nullptr, documentPath, 0);
				temp.resize(len);
				std::wcstombs(temp.data(), documentPath, len);

				path = temp;
				path /= "Arma 3";
			}
			CoTaskMemFree(documentPath);
		}

		// Folder = %profileFolder% (via commandline)
		else {
			path = profileFolder;
		}
#elif defined(__linux__)
		if (profileName.empty()) {
			profileName = "player";
		}

		std::filesystem::path exePath = process::tryGetExecutablePath();
		if (!exePath.empty()) {
			path = exePath / profileName;
		}
#endif
		if (path.empty()) {
			// Fall-back to mod folder
			std::filesystem::path dllFolder = tryGetDllFolder();
			if (std::filesystem::exists(dllFolder)) {
				path = dllFolder / "storage";
			}
			else {
				// Fall-back to current working directory
				char buffer[1024];
				if (getcwd(buffer, sizeof(buffer)) != NULL) {
					path = buffer;
				}
				else {
					// Throw it up to Arma process.
					throw std::runtime_error("FileXT: Fatal error, couldn't get a valid directory.");
				}
			}
		}

		return path;
	}

	const std::filesystem::path& getAndEnsureStorageDir() {
		// keep this static path internal so it doesn't get used too early or modified externally.
		static std::filesystem::path storage;

		if (storage.empty() || !std::filesystem::exists(storage)) {
			LOG_VERBOSE("File storage directory isn't set or doesn't exist. Attempting to find or create it now.");

			std::filesystem::path folder = tryGetStorageFolder();
			LOG_VERBOSE("Profile Folder: %s", folder.c_str());

			if (folder.empty() || !std::filesystem::exists(folder)) {
				LOG_CRITICAL("The server profile folder was not found: \"%s\". Attempting to fallback to mod folder.", folder.c_str());

				// Fall-back to mod folder
				folder = tryGetDllFolder();
				if (folder.empty() || !std::filesystem::exists(folder)) {
					LOG_CRITICAL("The FileXT mod folder was not found: \"%s\"", folder.c_str());
				}
			}

			storage = folder / "storage";

			if (!std::filesystem::exists(storage)) {
				LOG_VERBOSE("Storage folder not found, creating it now: \"%s\"", storage.c_str());
				std::filesystem::create_directory(storage);
			}

			if (!std::filesystem::exists(storage)) {
				LOG_CRITICAL("The file storage directory could not be created");
				storage.clear();
			}
		}

		return storage;
	}

	void tryMigrateStorageFolder() {
		static bool isMigrated = false;

		if (!isMigrated) {
			isMigrated = true;

			static const std::filesystem::path& dllFolder = tryGetDllFolder();
			static const std::filesystem::path& storage = getAndEnsureStorageDir();

			if (!std::filesystem::exists(dllFolder)) {
				LOG_CRITICAL("Could not find mod Dll folder in this environment.");
				return;
			}

			if (!std::filesystem::exists(storage)) {
				LOG_CRITICAL("Could not get storage folder in this environment.");
				return;
			}

			const std::filesystem::path oldStorage = dllFolder / "storage";

			// Migrate any files from deprecated storage
			if (std::filesystem::exists(oldStorage)) {
				for (const auto& dirEntry : std::filesystem::directory_iterator(oldStorage)) {
					const std::filesystem::path storageFile = storage / dirEntry.path().filename();
					if (!std::filesystem::exists(storageFile)) {
						std::filesystem::copy(dirEntry, storageFile);
					}
				}
			}
		}
	}

	std::filesystem::path& getAndEnsureLogPath() {
		static std::filesystem::path logPath;

		if (logPath.empty()) {
			logPath = tryGetStorageFolder() / "filext_log.log";
		}

		return logPath;
	}
}

namespace process
{
	std::filesystem::path tryGetExecutablePath() {
		std::filesystem::path exePath;
		char szPath[1024]; // MAX_PATH is no longer end-all be-all, but if your path is this big you deserve to suffer.
#if defined(WIN32)
		GetModuleFileNameA(NULL, szPath, 1024);
#elif defined(__linux__)
		readlink("/proc/self/exe", szPath, 1024);
#endif
		exePath = szPath;
		return exePath.parent_path();
	}

	std::vector<std::string> tryGetCommandLineArgs() {
		std::vector<std::string> args;

#ifdef WIN32
		LPWSTR* pszArgList;
		int n_args;
		int result;
		pszArgList = CommandLineToArgvW(GetCommandLineW(), &n_args);

		if (pszArgList != NULL) {
			for (int i = 0; i < n_args; i++) {
				args.emplace_back();

				LPWSTR arg = *(pszArgList + i);
				size_t len = wcstombs(nullptr, arg, 0);
				args.back().resize(len);
				std::wcstombs(args.back().data(), arg, len);
			}
		}

		LocalFree(pszArgList);
#elif defined(__linux__)
		// Taken from DTool library in Panda3D
		std::ifstream proc("/proc/self/cmdline");
		if (!proc.fail()) {
			int ch = proc.get();
			int index = 0;
			while (!proc.eof() && !proc.fail()) {
				std::string arg;

				while (!proc.eof() && !proc.fail() && ch != '\0') {
					arg += (char)ch;
					ch = proc.get();
				}

				if (index > 0) {
					args.push_back(arg);
				}
				index++;

				ch = proc.get();
			}
		}
#else
#pragma error("Not Implemented")
#endif

		return args;
	}
}