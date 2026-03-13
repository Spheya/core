#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <variant>
#include <vector>

#define MINIZ_NO_TIME
#define MINIZ_NO_DEFLATE_APIS
#define MINIZ_NO_ARCHIVE_WRITING_APIS
#define MINIZ_NO_ZLIB_APIS
#include "miniz/miniz.h"
#ifdef ZPACK_IMPLEMENTATION
	#include "miniz/miniz.c"
#endif

namespace zpack {

	class FileContent {
	public:
		FileContent() = default;
		FileContent(std::vector<char> data);

		const std::vector<char>& read();
		std::string readAsString();

	private:
		std::vector<char> m_data;
	};

	class ZipFileIterator {
	public:
		ZipFileIterator(mz_zip_archive* zip, mz_uint index);

		void increment();
		FileContent loadContent() const;
		std::filesystem::path path() const;

		bool operator==(const ZipFileIterator& other) const;
		bool operator!=(const ZipFileIterator& other) const;

	private:
		mz_zip_archive* m_zip;
		mz_uint m_index;
	};

	class DirFileIterator {
	public:
		explicit DirFileIterator(std::filesystem::recursive_directory_iterator iterator);

		void increment();
		FileContent loadContent() const;
		std::filesystem::path path() const;

		bool operator==(const DirFileIterator& other) const;
		bool operator!=(const DirFileIterator& other) const;

	private:
		std::filesystem::recursive_directory_iterator m_iterator;
	};

	class FileEntry {
		friend class FileIterator;

	public:
		FileEntry(std::variant<ZipFileIterator, DirFileIterator> iterator);

		FileContent loadContent() const;
		std::filesystem::path path() const;

	private:
		std::variant<ZipFileIterator, DirFileIterator> m_iterator;
	};

	class FileIterator {
	public:
		FileIterator(ZipFileIterator iterator);
		FileIterator(DirFileIterator iterator);

		FileIterator& operator++();
		FileIterator operator++(int);

		const FileEntry& operator*() const;
		const FileEntry* operator->() const;

		bool operator==(const FileIterator& other) const;
		bool operator!=(const FileIterator& other) const;

	private:
		FileEntry m_entry;
	};

	class ZipFileLoader {
	public:
		ZipFileLoader(const std::filesystem::path& path);
		ZipFileLoader(const void* memory, size_t size);
		ZipFileLoader(ZipFileLoader&) = delete;
		ZipFileLoader& operator=(ZipFileLoader&) = delete;
		ZipFileLoader(ZipFileLoader&& other) noexcept;
		ZipFileLoader& operator=(ZipFileLoader&& other) noexcept;
		~ZipFileLoader();

		FileIterator begin() const;
		FileIterator end() const;

		FileContent get(const std::filesystem::path& path) const;

		bool isValid() const;

	private:
		void destroy();

	private:
		mz_zip_archive* m_zip;
		char* m_memory;
	};

	class DirFileLoader {
	public:
		DirFileLoader(std::filesystem::path root);

		FileIterator begin() const;
		FileIterator end() const;

		FileContent get(const std::filesystem::path& path) const;

		bool isValid() const;

	private:
		std::filesystem::path m_root;
	};

	class FileLoader {
	public:
		FileLoader();
		explicit FileLoader(const std::filesystem::path& root);
		FileLoader(const void* packageMemory, size_t size);

		FileIterator begin() const;
		FileIterator end() const;

		FileContent get(const std::filesystem::path& path) const;

		bool isValid() const;

	private:
		std::variant<ZipFileLoader, DirFileLoader> m_loader;
	};

	inline ZipFileIterator::ZipFileIterator(mz_zip_archive* zip, mz_uint index) : m_zip(zip), m_index(index) {}

	inline void ZipFileIterator::increment() {
		++m_index;
	}

	inline FileContent::FileContent(std::vector<char> data) : m_data(std::move(data)) {}

	inline const std::vector<char>& FileContent::read() {
		return m_data;
	}

	inline std::string FileContent::readAsString() {
		return std::string(m_data.begin(), m_data.end());
	}

	inline FileContent ZipFileIterator::loadContent() const {
		size_t size;
		void* data = mz_zip_reader_extract_to_heap(m_zip, m_index, &size, 0);
		if(!data) return {};
		std::vector<char> vec(size);
		memcpy(vec.data(), data, size);
		mz_free(data);
		return std::move(vec);
	}

	inline std::filesystem::path ZipFileIterator::path() const {
		mz_uint size = mz_zip_reader_get_filename(m_zip, m_index, nullptr, 0);
		std::string fileName(size, 0);
		mz_zip_reader_get_filename(m_zip, m_index, fileName.data(), size);
		if(!fileName.empty() && fileName.back() == '\0') fileName.pop_back();
		return fileName;
	}

	inline bool ZipFileIterator::operator==(const ZipFileIterator& other) const {
		return m_zip == other.m_zip && m_index == other.m_index;
	}

	inline bool ZipFileIterator::operator!=(const ZipFileIterator& other) const {
		return m_zip != other.m_zip || m_index != other.m_index;
	}

	inline DirFileIterator::DirFileIterator(std::filesystem::recursive_directory_iterator iterator) : m_iterator(std::move(iterator)) {}

	inline void DirFileIterator::increment() {
		do ++m_iterator;
		while(m_iterator != std::filesystem::recursive_directory_iterator() && !m_iterator->is_regular_file());
	}

	inline FileContent DirFileIterator::loadContent() const {
		std::ifstream file(m_iterator->path(), std::ios::binary | std::ios::ate);
		if(!file.is_open()) return {};
		size_t fileSize = size_t(file.tellg());
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		if(!file.read(buffer.data(), fileSize)) return {};
		return std::move(buffer);
	}

	inline std::filesystem::path DirFileIterator::path() const {
		return m_iterator->path();
	}

	inline bool DirFileIterator::operator==(const DirFileIterator& other) const {
		return m_iterator == other.m_iterator;
	}

	inline bool DirFileIterator::operator!=(const DirFileIterator& other) const {
		return m_iterator != other.m_iterator;
	}

	inline FileEntry::FileEntry(std::variant<ZipFileIterator, DirFileIterator> iterator) : m_iterator(iterator) {}

	inline FileContent FileEntry::loadContent() const {
		return std::visit([](const auto& it) { return it.loadContent(); }, m_iterator);
	}

	inline std::filesystem::path FileEntry::path() const {
		return std::visit([](const auto& it) { return it.path(); }, m_iterator);
	}

	inline FileIterator::FileIterator(ZipFileIterator iterator) : m_entry(iterator) {}

	inline FileIterator::FileIterator(DirFileIterator iterator) : m_entry(iterator) {}

	inline FileIterator& zpack::FileIterator::operator++() {
		std::visit([](auto& it) { it.increment(); }, m_entry.m_iterator);
		return *this;
	}

	inline FileIterator zpack::FileIterator::operator++(int) {
		FileIterator tmp = *this;
		std::visit([](auto& it) { it.increment(); }, m_entry.m_iterator);
		return tmp;
	}

	inline const FileEntry& FileIterator::operator*() const {
		return m_entry;
	}

	inline const FileEntry* FileIterator::operator->() const {
		return &m_entry;
	}

	inline bool FileIterator::operator==(const FileIterator& other) const {
		return m_entry.m_iterator == other.m_entry.m_iterator;
	}

	inline bool FileIterator::operator!=(const FileIterator& other) const {
		return m_entry.m_iterator != other.m_entry.m_iterator;
	}

	inline ZipFileLoader::ZipFileLoader(const std::filesystem::path& path) : m_zip(new mz_zip_archive()), m_memory(nullptr) {
		*m_zip = {};

		if(!mz_zip_reader_init_file(m_zip, path.string().c_str(), 0)) {
			delete m_zip;
			m_zip = nullptr;
			return;
		}
	}

	inline ZipFileLoader::ZipFileLoader(const void* memory, size_t size) : m_zip(new mz_zip_archive()), m_memory(new char[size]) {
		*m_zip = {};
		memcpy(m_memory, memory, size);

		if(!mz_zip_reader_init_mem(m_zip, m_memory, size, 0)) {
			delete m_zip;
			m_zip = nullptr;
			return;
		}
	}

	inline ZipFileLoader::ZipFileLoader(ZipFileLoader&& other) noexcept : m_zip(other.m_zip), m_memory(other.m_memory) {
		other.m_zip = nullptr;
	}

	inline ZipFileLoader& ZipFileLoader::operator=(ZipFileLoader&& other) noexcept {
		destroy();
		m_zip = other.m_zip;
		m_memory = other.m_memory;
		other.m_zip = nullptr;
		return *this;
	}

	inline ZipFileLoader::~ZipFileLoader() {
		destroy();
	}

	inline FileIterator ZipFileLoader::begin() const {
		return FileIterator(ZipFileIterator(m_zip, 0));
	}

	inline FileIterator ZipFileLoader::end() const {
		return FileIterator(ZipFileIterator(m_zip, mz_zip_reader_get_num_files(m_zip)));
	}

	inline FileContent ZipFileLoader::get(const std::filesystem::path& path) const {
		mz_uint index;
		if(!mz_zip_reader_locate_file_v2(m_zip, path.string().c_str(), nullptr, 0, &index)) return {};
		return ZipFileIterator(m_zip, index).loadContent();
	}

	inline bool ZipFileLoader::isValid() const {
		return m_zip;
	}

	inline void ZipFileLoader::destroy() {
		if(m_zip) {
			mz_zip_reader_end(m_zip);
			delete m_zip;
			delete[] m_memory;
		}
	}

	inline DirFileLoader::DirFileLoader(std::filesystem::path root) : m_root(std::move(root)) {}

	inline FileIterator DirFileLoader::begin() const {
		return FileIterator(DirFileIterator(std::filesystem::recursive_directory_iterator(m_root)));
	}

	inline FileIterator DirFileLoader::end() const {
		return FileIterator(DirFileIterator(std::filesystem::recursive_directory_iterator()));
	}

	inline FileContent DirFileLoader::get(const std::filesystem::path& path) const {
		std::ifstream file(m_root / path, std::ios::binary | std::ios::ate);
		if(!file.is_open()) return {};
		size_t fileSize = size_t(file.tellg());
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		if(!file.read(buffer.data(), fileSize)) return {};
		return std::move(buffer);
	}

	inline bool DirFileLoader::isValid() const {
		return true;
	}

	inline FileLoader::FileLoader() : m_loader(DirFileLoader(".")) {}

	inline FileLoader::FileLoader(const std::filesystem::path& root) :
	    m_loader(
	        std::filesystem::is_directory(root) ? std::variant<ZipFileLoader, DirFileLoader>(DirFileLoader(root)) :
	                                              std::variant<ZipFileLoader, DirFileLoader>(ZipFileLoader(root))
	    ) {}

	inline FileLoader::FileLoader(const void* packageMemory, size_t size) : m_loader(ZipFileLoader(packageMemory, size)) {}

	inline FileIterator FileLoader::begin() const {
		return std::visit([](const auto& loader) { return loader.begin(); }, m_loader);
	}

	inline FileIterator FileLoader::end() const {
		return std::visit([](const auto& loader) { return loader.end(); }, m_loader);
	}

	inline FileContent FileLoader::get(const std::filesystem::path& path) const {
		return std::visit([&](const auto& loader) { return loader.get(path); }, m_loader);
	}

	inline bool FileLoader::isValid() const {
		return std::visit([](const auto& loader) { return loader.isValid(); }, m_loader);
	}

} // namespace zpack
