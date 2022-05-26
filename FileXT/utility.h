#pragma once

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

namespace string
{
	/// <summary>
	/// Trims the string of all /tokens/ from the start of the string to the first character that isn't in /tokens/.
	/// This functions modifies the original string.
	/// </summary>
	/// <param name="s">String to trim</param>
	/// <param name="tokens">String of individual characters to trim</param>
	inline void ltrim(std::string& s, std::string tokens) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [&tokens](unsigned char ch) {
			return tokens.find(ch) == std::string::npos;
			}));
	}

	/// <summary>
	/// Trims the string of all /tokens/ from the end of the string to the first character that isn't in /tokens/.
	/// This functions modifies the original string.
	/// </summary>
	/// <param name="s">String to trim</param>
	/// <param name="tokens">String of individual characters to trim</param>
	inline void rtrim(std::string& s, std::string tokens) {
		s.erase(std::find_if(s.rbegin(), s.rend(), [&tokens](unsigned char ch) {
			return tokens.find(ch) == std::string::npos;
			}).base(), s.end());
	}

	/// <summary>
	/// Trims the string of all /tokens/ from the start and end of the string to the first character on each side that isn't in /tokens/.
	/// This functions modifies the original string.
	/// </summary>
	/// <param name="s">String to trim</param>
	/// <param name="tokens">String of individual characters to trim</param>
	inline void trim(std::string& s, std::string tokens) {
		ltrim(s, tokens);
		rtrim(s, tokens);
	}

	/// <summary>
	/// Trims the string of all /tokens/ from the start of the string to the first character that isn't in /tokens/.
	/// This function creates a new string.
	/// </summary>
	/// <param name="s">String to trim</param>
	/// <param name="tokens">String of individual characters to trim</param>
	inline std::string trim(const char* szInput, std::string tokens) {
		std::string s(szInput);
		ltrim(s, tokens);
		rtrim(s, tokens);
		return s;
	}

	/// <summary>
	/// Checks if /token/ appears at the immediate start of /str/.
	/// This check is case-sensitive.
	/// </summary>
	/// <param name="str">The string to check for /token/.</param>
	/// <param name="token">The string to find.</param>
	/// <returns>True if /token/ appears at the immediate start of /str/. False otherwise.</returns>
	inline bool startsWith(const std::string& str, const std::string& token) {
		if (str.size() < token.size()) {
			return false;
		}

		for (int i = 0; i < token.size(); i++) {
			if (str.at(i) != token.at(i)) {
				return false;
			}
		}

		return true;
	}
}

namespace filext
{
	/// <summary>
	/// Attempts to get the folder that holds the Arma Server profile data.
	/// On failure to do so, will fall-back to:
	///		1. Default Arma Profile folder (Documents/Arma 3) - Windows only
	///		2. Default Arma Profile folder (Arma Folder/player) - Linux only
	///		3. Arma executable folder - Linux only
	///		4. FileXT Mod Folder (deprecated storage)
	///		5. Current working directory (usually Arma folder)
	/// </summary>
	/// <returns>Folder path for storage. Empty path on failure.</returns>
	std::filesystem::path tryGetStorageFolder();

	/// <summary>
	/// Attempts to get the parent folder of the FileXT DLL. (FileXT mod folder).
	/// </summary>
	/// <returns>Folder path of FileXT mod. Empty path on failure.</returns>
	const std::filesystem::path& tryGetDllFolder();

	/// <summary>
	/// Gets (and stores internally) a suitable location for storage data.
	/// </summary>
	/// <returns>Path to storage location.</returns>
	const std::filesystem::path& getAndEnsureStorageDir();

	/// <summary>
	/// Tries to migrate any files from the mod folder to the new profile folder.
	/// </summary>
	void tryMigrateStorageFolder();

	/// <summary>
	/// Gets (and stores internally) a suitable path to a log file. (Stored in storage directory).
	/// </summary>
	/// <returns></returns>
	std::filesystem::path& getAndEnsureLogPath();
}

namespace process
{
	/// <summary>
	/// Attempts to find the parent path of the current executing process. (Arma folder).
	/// </summary>
	/// <returns>Path to parent folder of current process. Empty on failure.</returns>
	std::filesystem::path tryGetExecutablePath();

	/// <summary>
	/// Attempts to get the command line arguments used to call this process.
	/// </summary>
	/// <returns>Contiguous list of elements of arguments used to call this process</returns>
	std::vector<std::string> tryGetCommandLineArgs();
}