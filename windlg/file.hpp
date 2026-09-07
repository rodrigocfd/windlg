#pragma once
#include <span>
#include "string.hpp"

namespace wd {

	/// Manages a file HANDLE.
	class File final {
	public:
		~File() noexcept { close(); }
		constexpr explicit File(HANDLE hFile = nullptr) noexcept : _hFile{hFile} { }

		File(File &&other) noexcept { operator=(std::forward<File>(other)); }
		File& operator=(File &&other) noexcept;

		File& close() noexcept;
		[[nodiscard]] HANDLE hfile() const noexcept { return _hFile; }
		[[nodiscard]] HANDLE leak() noexcept;
		File& open_read_only(StrView filePath);
		File& open_rw(StrView filePath);
		[[nodiscard]] std::vector<BYTE> read(size_t numBytes = std::dynamic_extent) const;
		[[nodiscard]] size_t size() const;
		FILETIME time_created() const;
		FILETIME time_last_write() const;
		File& truncate();
		File& write(std::span<const BYTE> src)    { return write(src.data(), src.size()); }
		File& write(const std::vector<BYTE> &src) { return write(src.data(), src.size()); }
		File& write(const BYTE *pSrc, size_t numBytes);

		template<std::input_iterator It>
		File& write(It first, It last) { return write(&(*first), std::distance(first, last)); }

	private:
		HANDLE _hFile;
	};

	/// File utilities.
	namespace file {
		[[nodiscard]] std::wstring dir_from(StrView filePath);      // Returns the directory, without trailing backslash.
		[[nodiscard]] std::wstring exe_dir();                       // Returns the directory of current EXE.
		[[nodiscard]] bool exists(StrView filePath) noexcept;       // Returns true if the filepath is valid, and the file exists.
		[[nodiscard]] std::wstring filename_from(StrView filePath); // Returns the name of the file, including its extension.
		[[nodiscard]] bool is_dir(StrView filePath) noexcept;       // Returns true if the path is valid, and is a directory.
		[[nodiscard]] std::vector<BYTE> read(StrView filePath);     // Reads the file contents as a BYTE blob.
		[[nodiscard]] std::wstring read_ini(StrView iniPath, StrView section, StrView key); // Reads an entry from an INI file as a string.
		[[nodiscard]] std::wstring remove_ext(StrView filePath); // Returns a new string truncating the dot and the extension.
		[[nodiscard]] size_t size(StrView filePath);             // Returns the file size in bytes.
		FILETIME time_created(StrView filePath);                 // Returns the creation date of the file.
		FILETIME time_last_write(StrView filePath);              // Returns the last write date of the file.
		void write(StrView filePath, const BYTE *pSrc, size_t numBytes); // Truncates the file and writes the new contents.
		inline void write(StrView filePath, std::span<const BYTE> src)    { write(filePath, src.data(), src.size()); } // Truncates the file and writes the new contents.
		inline void write(StrView filePath, const std::vector<BYTE> &src) { write(filePath, src.data(), src.size()); } // Truncates the file and writes the new contents.

		// Truncates the file and writes the new contents.
		template<typename It>
		void write(StrView filePath, It first, It last) { write(filePath, &(*first), std::distance(first, last)); }

		void write_ini(StrView iniPath, StrView section, StrView key, StrView val); // Writes a string to the entry of an INI file.
	}

	/// Iterates over all files and directories directly under a directory. Won't search recursively.
	class DirList final {
	public:
		/// DirList custom iterator, which is a view over the owning DirList object.
		class Iter final {
		public:
			using value_type = std::wstring;
			using difference_type = std::ptrdiff_t;
			using iterator_concept = std::input_iterator_tag;
			using iterator_category = std::input_iterator_tag;
			using reference = const std::wstring&;

			Iter& operator++();
			void operator++(int)                                                   { ++(*this); }
			[[nodiscard]] constexpr const std::wstring& operator*() const noexcept { return _curFullPath; }
			[[nodiscard]] bool operator==(const Iter &other) const noexcept;
		private:
			explicit Iter(DirList *pDirList);
			constexpr Iter() noexcept : _pDirList{nullptr}, _curFullPath{} { }
			void _check_finished_and_grab_wfd();

			DirList *_pDirList;
			std::wstring _curFullPath; // must be kept here because operator*() must be const, and returns a reference
			friend DirList;
		};

		DirList(const DirList&) = delete;
		DirList& operator=(const DirList&) = delete;
		DirList(DirList&&) = delete;
		DirList& operator=(DirList&&) = delete;

		~DirList() noexcept;
		explicit DirList(StrView dirPath);
		constexpr DirList() noexcept : _dirPath{}, _hFind{nullptr}, _wfd{}, _basePath{} { }

		[[nodiscard]] Iter begin() { return Iter{this}; }
		[[nodiscard]] Iter end()   { return Iter{}; }
	private:
		void _advance();

		std::wstring _dirPath;
		HANDLE _hFind;
		WIN32_FIND_DATAW _wfd;
		std::wstring _basePath;
	};

	/// Iterates over all files under a directory and its subdirectories, recursively.
	class DirListDeep final {
	public:
		/// DirList custom iterator, which is a view over the owning DirList object.
		class Iter final {
		public:
			using value_type = std::wstring;
			using difference_type = std::ptrdiff_t;
			using iterator_concept = std::input_iterator_tag;
			using iterator_category = std::input_iterator_tag;
			using reference = const std::wstring&;

			Iter& operator++();
			void operator++(int)                                                   { ++(*this); }
			[[nodiscard]] constexpr const std::wstring& operator*() const noexcept { return _curFullPath; }
			[[nodiscard]] bool operator==(const Iter &other) const noexcept;
		private:
			explicit Iter(DirListDeep *pDirListDeep);
			constexpr Iter() noexcept : _pDirListDeep{nullptr}, _curFullPath{} { }
			void _check_finished_and_grab_wfd();

			DirListDeep *_pDirListDeep;
			std::wstring _curFullPath; // must be kept here because operator*() must be const, and returns a reference
			friend DirListDeep;
		};

		DirListDeep(const DirListDeep&) = delete;
		DirListDeep& operator=(const DirListDeep&) = delete;
		DirListDeep(DirListDeep&&) = delete;
		DirListDeep& operator=(DirListDeep&&) = delete;

		~DirListDeep() noexcept;
		explicit DirListDeep(StrView dirPath);
		DirListDeep() noexcept : _flatList{}, _flatIter{_flatList.begin()}, _pDeepList{nullptr}, _pDeepIter{nullptr} { }

		[[nodiscard]] Iter begin() { return Iter{this}; }
		[[nodiscard]] Iter end()   { return Iter{}; }
	private:
		void _check_subfolder();
		void _advance();
		constexpr bool _finished() const                   { return _pDeepList ? *_pDeepIter == DirListDeep{}.end() : _flatIter == DirList{}.end(); }
		constexpr const std::wstring& _curFullPath() const { return _pDeepList ? **_pDeepIter : *_flatIter; }

		DirList _flatList;
		DirList::Iter _flatIter;
		DirListDeep *_pDeepList; // for recursive search
		DirListDeep::Iter *_pDeepIter;
	};

	/// Manages an INI file.
	class IniFile final {
	public:
		/// An entry of a Section of an IniFile.
		struct Entry final {
			std::wstring key{}, val{};
		};
		/// A section of an IniFile.
		struct Section final {
			std::wstring name{};
			std::vector<Entry> entries{}; // Entries of the INI file, freely editable.

			[[nodiscard]] const Entry* get(StrView keyName) const noexcept;                                                                // Returns nullptr if not found.
			[[nodiscard]] Entry* get(StrView keyName) noexcept           { return const_cast<Entry*>(std::as_const(*this).get(keyName)); } // Returns nullptr if not found.
			[[nodiscard]] bool has_entry(StrView keyName) const noexcept { return get(keyName) != nullptr; }
			[[nodiscard]] int read_int(StrView keyName) const            { return std::stoi(read_str(keyName)); }              // Throws std::out_of_range if not found.
			[[nodiscard]] std::wstring read_str(StrView keyName) const;                                                        // Throws std::out_of_range if not found.
			[[nodiscard]] UINT read_uint(StrView keyName) const          { return std::stoul(read_str(keyName)); }             // Throws std::out_of_range if not found.
			Section& write(StrView keyName, int val)                     { return _raw_write(keyName, std::to_wstring(val)); } // Creates a new entry if not found.
			Section& write(StrView keyName, StrView val)                 { return _raw_write(keyName, std::wstring{val}); }    // Creates a new entry if not found.
			Section& write(StrView keyName, UINT val)                    { return _raw_write(keyName, std::to_wstring(val)); } // Creates a new entry if not found.
		private:
			Section& _raw_write(StrView keyName, std::wstring &&val);
		};

		std::vector<Section> sections{}; // Sections of the INI file, freely editable.

		IniFile() = default;
		explicit IniFile(StrView iniPath) { load(iniPath); }

		IniFile& load(StrView iniPath);
		IniFile& save(StrView lineBreak = L"\r\n")                                        { return save_as(_iniPath, lineBreak); } // Saves the INI to the same file it was loaded from.
		IniFile& save_as(StrView newIniPath, StrView lineBreak = L"\r\n");                                                         // Saves the INI file to a different file. The internal path will be updated, so a subsequent save() will use the new path.
		[[nodiscard]] const Section* get(StrView sectionName) const noexcept;
		[[nodiscard]] Section* get(StrView sectionName) noexcept                          { return const_cast<Section*>(std::as_const(*this).get(sectionName)); }
		[[nodiscard]] const Entry* get(StrView sectionName, StrView keyName) const noexcept;
		[[nodiscard]] Entry* get(StrView sectionName, StrView keyName) noexcept           { return const_cast<Entry*>(std::as_const(*this).get(sectionName, keyName)); }
		[[nodiscard]] bool has_entry(StrView sectionName, StrView keyName) const noexcept { return get(sectionName, keyName) != nullptr; }
		[[nodiscard]] bool has_section(StrView sectionName) const noexcept                { return get(sectionName) != nullptr; }
		[[nodiscard]] constexpr const StrView ini_path() const noexcept                   { return _iniPath; } // Returns the internal path, which is where the INI will be written on save().
	private:
		std::wstring _iniPath{};
	};

	/// Version information from the EXE.
	struct VersionInfo final {
	public:
		VersionInfo();
		explicit VersionInfo(StrView exePath);

		WORD version[4] = {0};
		std::wstring comments{};
		std::wstring companyName{};
		std::wstring fileDescription{};
		std::wstring fileVersion{};
		std::wstring internalName{};
		std::wstring legalCopyright{};
		std::wstring legalTrademarks{};
		std::wstring originalFilename{};
		std::wstring productName{};
		std::wstring productVersion{};
		std::wstring privateBuild{};
		std::wstring specialBuild{};
	};

}
