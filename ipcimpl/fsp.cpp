#include "Ctu.h"
#include <dirent.h>

/*$IPC$
partial nn::fssrv::sf::IFile {
	[ctor] string fn;
	[ctor] uint32_t mode;
	void *fp;
	bool isOpen;
	long bufferOffset;
}

partial nn::fssrv::sf::IFileSystem {
	[ctor] string fnPath;
}

partial nn::fssrv::sf::IStorage {
	[ctor] string fn;
	void *fp;
	bool isOpen;
	long bufferOffset;
}
partial nn::fssrv::sf::IDirectory {
	[ctor] string fn;
	[ctor] uint32_t filter;
	void *fp;
	bool isOpen;
}
*/

/* ---------------------------------------- Start of IFileSystem ---------------------------------------- */
// Interface
nn::fssrv::sf::IStorage::IStorage(Ctu *_ctu, string  _fn) : IpcService(_ctu), fn(_fn) {
	fn = "SwitchFS/" + _fn;
	LOG_DEBUG(Fsp, "Open IStorage \"%s\"", fn.c_str());
	fp = fopen(fn.c_str(), "r+");
	if(fp) {
		isOpen = true;
	} else {
		LOG_DEBUG(Fsp, "FILE NOT FOUND!");
		isOpen = false;
	}

}

uint32_t nn::fssrv::sf::IStorage::Flush() {
	if(isOpen && fp != nullptr)
		fflush((FILE *)fp);
	return 0;
}
uint32_t nn::fssrv::sf::IStorage::GetSize(OUT uint64_t& size) {
	if(isOpen && fp != nullptr) {
		bufferOffset = ftell((FILE *)fp);
		fseek((FILE *)fp, 0, SEEK_END);
		long fSize = ftell((FILE *)fp);
		fseek((FILE *)fp, bufferOffset, SEEK_SET);
		size = fSize;
		return 0;
	}
	LOG_DEBUG(Fsp, "Failed to get file size!");
	return 0;
}
uint32_t nn::fssrv::sf::IStorage::Read(IN uint64_t offset, IN uint64_t length, OUT int8_t * buffer, guint buffer_size) {
	if(isOpen && fp != nullptr) {
		uint32_t s = ((uint32_t)buffer_size < (uint32_t)length ? (uint32_t)buffer_size : (uint32_t)length);
		bufferOffset = offset;
		fseek((FILE *)fp, offset, SEEK_SET);
		fread(buffer, 1, s, (FILE *)fp);
		bufferOffset = ftell((FILE *)fp);
	}
	return 0;
}
uint32_t nn::fssrv::sf::IStorage::SetSize(IN uint64_t size) {
	if(isOpen && fp != nullptr) {
		fseek((FILE *)fp, 0, SEEK_END);
		uint32_t curSize = (uint32_t)ftell((FILE *)fp);

		if(curSize < (uint32_t)size) {
			uint32_t remaining = (uint32_t)size-curSize;
			char *buf = (char*)malloc(remaining);
			memset(buf, 0, remaining);
			fwrite(buf, 1, remaining, (FILE *)fp);
			free(buf);
		}

		fseek((FILE *)fp, bufferOffset, SEEK_SET);
	}
	return 0;
}
uint32_t nn::fssrv::sf::IStorage::Write(IN uint64_t offset, IN uint64_t length, IN int8_t * data, guint data_size) {
	if(isOpen && fp != nullptr) {
		bufferOffset = offset;
		uint32_t s = ((uint32_t)data_size < (uint32_t)length ? (uint32_t)data_size : (uint32_t)length);
		fseek((FILE *)fp, offset, SEEK_SET);
		fwrite(data, 1, s, (FILE *)fp);
		if(length-s > 0) {
			char *buf2 = (char*)malloc(length-s);
			memset(buf2, 0, length-s);
			fwrite(buf2, 1, length-s, (FILE *)fp);
			free(buf2);
		}
		bufferOffset = ftell((FILE *)fp);

	}
	return 0;
}

// Funcs
uint32_t nn::fssrv::sf::IFileSystemProxy::OpenBisPartition(IN nn::fssrv::sf::Partition partitionID, OUT shared_ptr<nn::fssrv::sf::IStorage>& BisPartition) {
	LOG_DEBUG(Fsp, "Stub implementation for nn::fssrv::sf::IFileSystemProxy::OpenBisPartition");
	BisPartition = buildInterface(nn::fssrv::sf::IStorage, "bis.istorage");
	return 0x0;
}

uint32_t nn::fssrv::sf::IFileSystemProxy::OpenDataStorageByApplicationId(IN nn::ApplicationId tid, OUT shared_ptr<nn::fssrv::sf::IStorage>& dataStorage) {
	LOG_DEBUG(Fsp, "Stub implementation for nn::fssrv::sf::IFileSystemProxy::OpenDataStorageByApplicationId 0x" ADDRFMT, tid);
	std::stringstream ss;
	ss << "tid_archives_" << hex << tid << ".istorage";
	dataStorage = buildInterface(nn::fssrv::sf::IStorage, ss.str());
	return 0;
}

uint32_t nn::fssrv::sf::IFileSystemProxy::OpenDataStorageByCurrentProcess(OUT shared_ptr<nn::fssrv::sf::IStorage>& dataStorage) {
	LOG_DEBUG(Fsp, "Stub implementation for nn::fssrv::sf::IFileSystemProxy::OpenDataStorageByCurrentProcess");
	LOG_ERROR(Fsp, "UNIMPLEMENTED!!!");
	dataStorage = buildInterface(nn::fssrv::sf::IStorage, "");
	return 0;
}

uint32_t nn::fssrv::sf::IFileSystemProxy::OpenDataStorageByDataId(IN nn::ApplicationId tid, IN uint8_t storageId, OUT shared_ptr<nn::fssrv::sf::IStorage>& dataStorage) {
	LOG_DEBUG(Fsp, "Stub implementation for nn::fssrv::sf::IFileSystemProxy::OpenDataStorageByDataId 0x" ADDRFMT, 0x0100000000000800+(uint64_t)storageId);
	std::stringstream ss;
	ss << "archives/" << hex << setw(16) << setfill('0') << 0x0100000000000800+(uint64_t)storageId << ".istorage";
	dataStorage = buildInterface(nn::fssrv::sf::IStorage, ss.str());
	return 0;
}

uint32_t nn::fssrv::sf::IFileSystemProxy::OpenGameCardPartition(IN nn::fssrv::sf::Partition partitionID, IN uint32_t _1, OUT shared_ptr<nn::fssrv::sf::IStorage>& gameCardFs) {
	LOG_DEBUG(Fsp, "Stub implementation for nn::fssrv::sf::IFileSystemProxy::OpenGameCardPartition");
	gameCardFs = buildInterface(nn::fssrv::sf::IStorage, "GamePartition.istorage");
	return 0;
}

uint32_t nn::fssrv::sf::IFileSystemProxy::OpenRomStorage(OUT shared_ptr<nn::fssrv::sf::IStorage>& _0) {
	LOG_DEBUG(Fsp, "Stub implementation for nn::fssrv::sf::IFileSystemProxy::OpenRomStorage");
	_0 = buildInterface(nn::fssrv::sf::IStorage, "RomStorage.istorage");
	return 0;
}

/* ---------------------------------------- End of IStorage ---------------------------------------- */

/* ---------------------------------------- Start of IFileSystem ---------------------------------------- */
// Interface
nn::fssrv::sf::IFileSystem::IFileSystem(Ctu *_ctu, string  _fnPath) : IpcService(_ctu), fnPath(_fnPath) {
	fnPath = "SwitchFS/" + _fnPath;
	LOG_DEBUG(Fsp, "Open path %s", fnPath.c_str());
}

uint32_t nn::fssrv::sf::IFileSystem::DeleteFile(IN int8_t * path, guint path_size) {
	LOG_DEBUG(Fsp, "Delete file %s", (fnPath+string((char*)path)).c_str());
	remove((fnPath+string((char*)path)).c_str());
	return 0;
}

uint32_t nn::fssrv::sf::IFileSystem::CreateFile(IN uint64_t mode, IN uint32_t size, IN int8_t * path, guint path_size) {
	LOG_DEBUG(Fsp, "Create file %s", (fnPath+string((char*)path)).c_str());
	FILE *fp = fopen((fnPath+string((char*)path)).c_str(), "wb");
	if(!fp)
		return 0x7d402;
	fclose(fp);
	return 0;
}

uint32_t nn::fssrv::sf::IFileSystem::CreateDirectory(IN int8_t * path, guint path_size) {
	LOG_DEBUG(Fsp, "Create directory %s", (fnPath+string((char*)path)).c_str());
	if (mkdir((fnPath+string((char*)path)).c_str(), 0755) == -1) {
		if (errno == EEXIST)
			return 0x402;
		else
			return 0x7d402;
	}
	return 0;
}

uint32_t nn::fssrv::sf::IFileSystem::GetEntryType(IN int8_t * path, guint path_size, OUT uint32_t& _1) {
	LOG_DEBUG(IpcStubs, "GetEntryType for file %s", (fnPath + string((char*)path)).c_str());
	struct stat path_stat;

	stat((fnPath+string((char*)path)).c_str(), &path_stat);
	if (S_ISREG(path_stat.st_mode))
		_1 = 1;
	else if (S_ISDIR(path_stat.st_mode))
		_1 = 0;
	else
		return 0x271002;
	return 0;
}


// Funcs
uint32_t nn::fssrv::sf::IFileSystemProxy::OpenDataFileSystemByCurrentProcess(OUT shared_ptr<nn::fssrv::sf::IFileSystem>& _0) {
	LOG_DEBUG(Fsp, "Stub implementation for nn::fssrv::sf::IFileSystemProxy::OpenDataFileSystemByCurrentProcess");
	_0 = buildInterface(nn::fssrv::sf::IFileSystem, "");
	return 0;
}

uint32_t nn::fssrv::sf::IFileSystemProxy::MountContent7(IN nn::ApplicationId tid, IN uint32_t ncaType, OUT shared_ptr<nn::fssrv::sf::IFileSystem>& _2) {
	LOG_DEBUG(Fsp, "Stub implementation for nn::fssrv::sf::IFileSystemProxy::MountContent7");
	_2 = buildInterface(nn::fssrv::sf::IFileSystem, "");
	return 0;
}

uint32_t nn::fssrv::sf::IFileSystemProxy::MountContent(IN nn::ApplicationId tid, IN uint32_t flag, IN int8_t * path, guint path_size, OUT shared_ptr<nn::fssrv::sf::IFileSystem>& contentFs) {
	LOG_DEBUG(Fsp, "Stub implementation for nn::fssrv::sf::IFileSystemProxy::MountContent");
	contentFs = buildInterface(nn::fssrv::sf::IFileSystem, "");
	return 0;
}

uint32_t nn::fssrv::sf::IFileSystemProxy::OpenDataFileSystemByApplicationId(IN nn::ApplicationId tid, OUT shared_ptr<nn::fssrv::sf::IFileSystem>& dataFiles) {
	LOG_DEBUG(Fsp, "Stub implementation for nn::fssrv::sf::IFileSystemProxy::OpenDataFileSystemByApplicationId");
	dataFiles = buildInterface(nn::fssrv::sf::IFileSystem, "");
	return 0;
}

uint32_t nn::fssrv::sf::IFileSystemProxy::MountBis(IN nn::fssrv::sf::Partition partitionID, IN int8_t * path, guint path_size, OUT shared_ptr<nn::fssrv::sf::IFileSystem>& Bis) {
	LOG_DEBUG(Fsp, "Stub implementation for nn::fssrv::sf::IFileSystemProxy::MountBis");
	Bis = buildInterface(nn::fssrv::sf::IFileSystem, string("BIS/") + to_string(partitionID));
	return 0;
}

uint32_t nn::fssrv::sf::IFileSystemProxy::OpenHostFileSystemImpl(IN int8_t * path, guint path_size, OUT shared_ptr<nn::fssrv::sf::IFileSystem>& _1) {
	LOG_DEBUG(Fsp, "Stub implementation for nn::fssrv::sf::IFileSystemProxy::OpenHostFileSystemImpl");
	_1 = buildInterface(nn::fssrv::sf::IFileSystem, "");
	return 0;
}

uint32_t nn::fssrv::sf::IFileSystemProxy::MountSdCard(OUT shared_ptr<nn::fssrv::sf::IFileSystem>& sdCard) {
	LOG_DEBUG(Fsp, "Stub implementation for nn::fssrv::sf::IFileSystemProxy::MountSdCard");
	sdCard = buildInterface(nn::fssrv::sf::IFileSystem, "SDCard");
	return 0;
}

uint32_t nn::fssrv::sf::IFileSystemProxy::MountGameCardPartition(IN uint32_t _0, IN uint32_t _1, OUT shared_ptr<nn::fssrv::sf::IFileSystem>& gameCardPartitionFs) {
	LOG_DEBUG(Fsp, "Stub implementation for nn::fssrv::sf::IFileSystemProxy::MountGameCardPartition");
	gameCardPartitionFs = buildInterface(nn::fssrv::sf::IFileSystem, "GameCard");
	return 0;
}

uint32_t nn::fssrv::sf::IFileSystemProxy::MountSaveData(IN uint8_t input, IN nn::fssrv::sf::SaveStruct saveStruct, OUT shared_ptr<nn::fssrv::sf::IFileSystem>& saveDataFs) {
	uint64_t tid = *(uint64_t *)(&saveStruct[0x18]);
	LOG_DEBUG(Fsp, "Stub implementation for nn::fssrv::sf::IFileSystemProxy::MountSaveData 0x" ADDRFMT, tid);
	std::stringstream ss;
	ss << "save_" << hex << tid;
	saveDataFs = buildInterface(nn::fssrv::sf::IFileSystem, ss.str());
	return 0;
}

uint32_t nn::fssrv::sf::IFileSystemProxy::MountSystemSaveData(IN uint8_t input, IN nn::fssrv::sf::SaveStruct saveStruct, OUT shared_ptr<nn::fssrv::sf::IFileSystem>& systemSaveDataFs) {
	uint64_t tid = *(uint64_t *)(&saveStruct[0x18]);
	LOG_DEBUG(Fsp, "Stub implementation for nn::fssrv::sf::IFileSystemProxy::MountSystemSaveData 0x" ADDRFMT, tid);
	std::stringstream ss;
	ss << "syssave_" << hex << tid;
	systemSaveDataFs = buildInterface(nn::fssrv::sf::IFileSystem, ss.str());
	return 0;
}

uint32_t nn::fssrv::sf::IFileSystemProxy::MountSaveDataReadOnly(IN uint8_t input, IN nn::fssrv::sf::SaveStruct saveStruct, OUT shared_ptr<nn::fssrv::sf::IFileSystem>& saveDataFs) {
	uint64_t tid = *(uint64_t *)(&saveStruct[0x18]);
	LOG_DEBUG(Fsp, "Stub implementation for nn::fssrv::sf::IFileSystemProxy::MountSaveDataReadOnly 0x" ADDRFMT, tid);
	std::stringstream ss;
	ss << "save_" << hex << tid;
	saveDataFs = buildInterface(nn::fssrv::sf::IFileSystem, ss.str());
	return 0;
}

uint32_t nn::fssrv::sf::IFileSystemProxy::MountImageDirectory(IN uint32_t _0, OUT shared_ptr<nn::fssrv::sf::IFileSystem>& imageFs) {
	LOG_DEBUG(Fsp, "Stub implementation for nn::fssrv::sf::IFileSystemProxy::MountImageDirectory");
	imageFs = buildInterface(nn::fssrv::sf::IFileSystem, string("Image_") + to_string(_0));
	return 0;
}

uint32_t nn::fssrv::sf::IFileSystemProxy::MountContentStorage(IN uint32_t contentStorageID, OUT shared_ptr<nn::fssrv::sf::IFileSystem>& contentFs) {
	LOG_DEBUG(Fsp, "Stub implementation for nn::fssrv::sf::IFileSystemProxy::MountContentStorage");
	contentFs = buildInterface(nn::fssrv::sf::IFileSystem, string("CS_") + to_string(contentStorageID));
	return 0;
}

uint32_t nn::fssrv::sf::IFileSystemProxyForLoader::MountCode(IN nn::ApplicationId TID, IN int8_t * contentPath, guint contentPath_size, OUT shared_ptr<nn::fssrv::sf::IFileSystem>& contentFs) {
	LOG_DEBUG(Fsp, "Stub implementation for nn::fssrv::sf::IFileSystemProxyForLoader::MountCode");
	contentFs = buildInterface(nn::fssrv::sf::IFileSystem, "");
	return 0;
}

/* ---------------------------------------- End of IFileSystem ---------------------------------------- */

/* ---------------------------------------- Start of IDirectory ----------------------------------- */
// Interface
nn::fssrv::sf::IDirectory::IDirectory(Ctu *_ctu, string  _fn, uint32_t  _filter) : IpcService(_ctu), fn(_fn), filter(_filter) {
	LOG_DEBUG(Fsp, "IDirectory: Directory path \"%s\"", fn.c_str());
	fp = opendir(fn.c_str());
	if(fp) {
		isOpen = true;
	} else {
		LOG_DEBUG(Fsp, "DIRECTORY NOT FOUND!");
		isOpen = false;
	}
}

struct DirectoryEntry {
	char path[0x300];
	uint32_t unk1;
	uint8_t entry_type;
	uint8_t pad[3];
	uint64_t filesize;
};

static_assert(sizeof(DirectoryEntry) == 0x310);

uint32_t nn::fssrv::sf::IDirectory::Read(OUT uint64_t& entries_read, OUT uint8_t * entries_buf, guint entries_buf_len) {
	size_t entries_count = entries_buf_len / sizeof(DirectoryEntry);
	LOG_DEBUG(Fsp, "IDirectory::Read: Attempting to read %zu entries (from %zu)", entries_count, entries_buf_len);
	struct DirectoryEntry *entries = (struct DirectoryEntry*)entries_buf;
	struct dirent *curdir;
	struct stat curdir_stat;
	uint64_t i;

	for (i = 0; i < entries_count; i++) {
		curdir = readdir((DIR*)fp);
		if (curdir == nullptr)
			break;
		strcpy(entries[i].path, curdir->d_name);
		entries[i].unk1 = 0;
		entries[i].entry_type = curdir->d_type == DT_DIR ? 0 : 1;
		if (stat((fn + std::string("/") + std::string(curdir->d_name)).c_str(), &curdir_stat) == -1) {
			LOG_DEBUG(Fsp, "We got an error getting size of %s", curdir->d_name);
			perror("stat");
			return 0x271002;/* error out */
		}
		entries[i].filesize = curdir_stat.st_size;
	}
	entries_read = i;
	return 0;
}


/* ---------------------------------------- End of IDirectory ---------------------------------------- */

/* ---------------------------------------- Start of IFile ---------------------------------------- */
// Interface
nn::fssrv::sf::IFile::IFile(Ctu *_ctu, string  _fn, uint32_t  _mode) : IpcService(_ctu), fn(_fn), mode(_mode) {
	LOG_DEBUG(Fsp, "IFile: File path \"%s\"", fn.c_str());
	string openModes[] = {"", "rb", "wb+", "wb+", "ab+", "ab+", "ab+", "ab+"};
	fp = fopen(fn.c_str(), openModes[_mode].c_str());
	if(fp) {
		isOpen = true;
	} else {
		LOG_DEBUG(Fsp, "FILE NOT FOUND!");
		isOpen = false;
	}
}

nn::fssrv::sf::IFile::~IFile() {
	fclose((FILE*)fp);
}

uint32_t nn::fssrv::sf::IFile::GetSize(OUT uint64_t& fileSize) {
	if(isOpen && fp != nullptr) {
		bufferOffset = ftell((FILE *)fp);
		fseek((FILE *)fp, 0, SEEK_END);
		long fSize = ftell((FILE *)fp);
		fseek((FILE *)fp, bufferOffset, SEEK_SET);
		fileSize = fSize;
		return 0;
	}
	LOG_DEBUG(Fsp, "Failed to get file size!");
	return 0;
}

uint32_t nn::fssrv::sf::IFile::Read(IN uint64_t _0, IN uint64_t offset, IN uint32_t size, OUT uint64_t& out_size, OUT int8_t * out_buf, guint out_buf_size) {
	LOG_DEBUG(Fsp, "IFile::Read from %s from %lu", fn.c_str(), offset);
	if(isOpen && fp != nullptr) {
		uint32_t s = ((uint32_t)out_buf_size < size ? (uint32_t)out_buf_size : size);
		bufferOffset = offset;
		fseek((FILE *)fp, offset, SEEK_SET);
		size_t s_read = fread(out_buf, 1, s, (FILE *)fp);
		bufferOffset = ftell((FILE *)fp);
		out_size = (uint32_t)s_read;
	} else {
		LOG_DEBUG(Fsp, "File is closed !");
	}
	return 0x0;
}

uint32_t nn::fssrv::sf::IFile::Write(IN uint64_t _0, IN uint64_t offset, IN uint32_t size, IN int8_t * buf, guint buf_size) {
	LOG_DEBUG(Fsp, "Stub implementation for nn::fssrv::sf::IFile::Write");
	if(isOpen && fp != nullptr) {
		bufferOffset = offset;
		uint32_t s = ((uint32_t)buf_size < size ? (uint32_t)buf_size : size);
		fseek((FILE *)fp, offset, SEEK_SET);
		fwrite(buf, 1, s, (FILE *)fp);
		if(size-s > 0) {
			char *buf2 = (char*)malloc(size-s);
			memset(buf2, 0, size-s);
			fwrite(buf2, 1, size-s, (FILE *)fp);
			free(buf2);
		}
		bufferOffset = ftell((FILE *)fp);

	}
	return 0;
}

uint32_t nn::fssrv::sf::IFile::Flush() {
	if(isOpen && fp != nullptr)
		fflush((FILE *)fp);
	return 0;
}

#ifdef _WIN32
#include <io.h>
static int truncate_file(FILE *f, off_t length) {
	return _chsize(fileno(f), length);
}
#elif __unix__
static int truncate_file(FILE *f, off_t length) {
	return ftruncate(fileno(f), length);
}
#else
#error "Unknown OS"
#endif

uint32_t nn::fssrv::sf::IFile::SetSize(IN uint64_t size) {
	if(isOpen && fp != nullptr) {
		truncate_file((FILE*)fp, (off_t)size);
	}
	return 0;
}

// Funcs
uint32_t nn::fssrv::sf::IFileSystem::OpenFile(IN uint32_t mode, IN int8_t * path, guint path_size, OUT shared_ptr<nn::fssrv::sf::IFile>& file) {
	LOG_DEBUG(Fsp, "OpenFile %s", path);
	auto tempi = buildInterface(nn::fssrv::sf::IFile, fnPath + "/" + string((char*)path), mode);
	if(tempi->isOpen) {
		file = tempi;
		return 0;
	} else
		return 0x7d402;
}

uint32_t nn::fssrv::sf::IFileSystem::OpenDirectory(IN uint32_t filter, IN int8_t * path, guint path_size, OUT shared_ptr<nn::fssrv::sf::IDirectory>& dir) {
	LOG_DEBUG(Fsp, "OpenDirectory %s", path);
	auto tempi = buildInterface(nn::fssrv::sf::IDirectory, fnPath + "/" + string((char*)path), filter);
	if(tempi->isOpen) {
		dir = tempi;
		return 0;
	} else
		return 0x7d402;
}

uint32_t nn::fssrv::sf::IFileSystemProxy::OpenSaveDataThumbnailFile(IN uint8_t _0, IN uint8_t * _1, IN uint32_t _2, OUT shared_ptr<nn::fssrv::sf::IFile>& thumbnail) {
	LOG_DEBUG(Fsp, "Stub implementation for nn::fssrv::sf::IFileSystemProxy::OpenSaveDataThumbnailFile");
	thumbnail = buildInterface(nn::fssrv::sf::IFile, string((char*)_1), _0);
	return 0;
}


/* ---------------------------------------- End of IFile ---------------------------------------- */


uint32_t nn::fssrv::sf::IEventNotifier::Unknown0(OUT shared_ptr<KObject>& _0) {
	LOG_DEBUG(IpcStubs, "Stub implementation for nn::fssrv::sf::IEventNotifier::Unknown0");
	_0 = make_shared<Waitable>();
	return 0;
}
