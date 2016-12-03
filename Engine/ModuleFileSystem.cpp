#include "ModuleFileSystem.h"

#include "Application.h"

#include "PhysFS/include/physfs.h"

#include "Assimp/include/cfileio.h"
#include "Assimp/include/types.h"

#include <sys/stat.h>
#include <time.h>

#pragma comment( lib, "PhysFS/libx86/physfs.lib" )

ModuleFileSystem::ModuleFileSystem(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	moduleName = "ModuleFileSystem";

	// need to be created before Awake so other modules can use it
	char* base_path = SDL_GetBasePath();
	PHYSFS_init(base_path);
	SDL_free(base_path);

	// By default we include executable's own directory
	// without this we won't be able to find config.xml :-(
	AddPath(".");
}

// Destructor
ModuleFileSystem::~ModuleFileSystem()
{
	PHYSFS_deinit();
}

// Called before render is available
//bool ModuleFileSystem::Awake(pugi::xml_node& config)
bool ModuleFileSystem::Init()
{

	LOG("Loading File System");
	bool ret = true;

	// Add all paths in configuration in order
//	for(pugi::xml_node path = config.child("path"); path; path = path.next_sibling("path"))
//	{
//		AddPath(path.child_value());
//	}

	char* tmp = SDL_GetBasePath();
	std::string writePath(tmp);

	if (PHYSFS_setWriteDir(writePath.data()) == 0)
	{
		LOG("File System error while creating write dir: %s", PHYSFS_getLastError());
	}
	else
	{
		// We add the writing directory as a reading directory too with speacial mount point
		LOG("Writing directory is %s", writePath.data());
		AddPath(writePath.data());
	}

	SDL_free(tmp);

	return ret;
}

// Called before quitting
bool ModuleFileSystem::CleanUp()
{
	//LOG("Freeing File System subsystem");
	if (AssimpIO)
	{
		delete AssimpIO;
	}

	return true;
}

// Add a new zip file or folder
bool ModuleFileSystem::AddPath(const char* path_or_zip, const char* mount_point)
{
	bool ret = false;

	if (PHYSFS_mount(path_or_zip, mount_point, 1) == 0)
	{
		LOG("File System error while adding a path or zip(%s): %s", path_or_zip, PHYSFS_getLastError());
	}
	else
	{
		ret = true;
	}

	return ret;
}

// Check if a file exists
bool ModuleFileSystem::Exists(const char* file) const
{
	return PHYSFS_exists(file) != 0;
}

bool ModuleFileSystem::CreateDir(const char * dir)
{
	if (IsDirectory(dir) == false)
	{
		PHYSFS_mkdir(dir);
		return true;
	}
	return false;
}

bool ModuleFileSystem::DelDir(const char * dir)
{
	if (IsDirectory(dir) == true)
	{
		std::vector<std::string> folders;
		std::vector<std::string> files;
		GetFilesIn(dir, &folders, &files);

		for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); it++)
		{
			std::string filePath(dir);
			filePath += "/";
			filePath += *it;
			DelFile(filePath.data());
		}
		for (std::vector<std::string>::iterator it = folders.begin(); it != folders.end(); it++)
		{
			std::string dirPath(dir);
			dirPath += "/";
			dirPath += *it;
			DelDir(dirPath.data());
		}
		if (PHYSFS_delete(dir) == 0)
		{
			LOG("Error deleting directory %s.\n%s\n", dir, PHYSFS_getLastError());
			return false;
		}
		LOG("Succesfully deleted directory %s", dir);
		return true;
	}
	LOG("Could not delete %s\nIt does not exist or could not be found", dir);
	return false;
}

bool ModuleFileSystem::DelFile(const char * file)
{
	if (Exists(file) == true)
	{
		if (PHYSFS_delete(file) != 0)
		{
			return true;
		}
		LOG("Error deleting file %s.\n%s\n", file, PHYSFS_getLastError());
	}
	else
	{
		LOG("Error deleting file %s.\n Could not be found or doesn't exist", file);
	}
	return false;
}

// Check if a file is a directory
bool ModuleFileSystem::IsDirectory(const char* file) const
{
	return PHYSFS_isDirectory(file) != 0;
}

std::string ModuleFileSystem::GetWrittingDirectory()
{
	return std::string(PHYSFS_getWriteDir());
}

void ModuleFileSystem::GetFilesIn(const char * directory, std::vector<std::string>* folders, std::vector<std::string>* files)
{
	char** f = PHYSFS_enumerateFiles(directory);
	char** it;

	for (it = f; *it != nullptr; it++)
	{
		std::string toPush(*it);
		if (GetFileFormat(*it).length() > 0)
		{
			files->push_back(toPush);
		}
		else
		{
			folders->push_back(toPush);
		}
	}

	PHYSFS_freeList(f);
}

Date ModuleFileSystem::ReadFileDate(const char * path)
{
	struct tm *foo;
	struct stat attrib;

	stat(path, &attrib);
	foo = gmtime(&(attrib.st_mtime));

	Date date;
	date.year = foo->tm_year;
	date.month = foo->tm_mon;
	date.day = foo->tm_mday;
	date.hour = foo->tm_hour;
	date.min = foo->tm_min;
	date.sec = foo->tm_sec;

	return date;
}


bool ModuleFileSystem::EraseFile(const char* file)
{
	if (true || Exists(file))
	{
		if (PHYSFS_delete(file) != 0)
		{
			return true;
		}
		LOG("Tried to erase %s, error: %s", file, PHYSFS_getLastError());
		return false;
	}
	LOG("Tried to erase %s, which could not be found", file);
	return false;
}


unsigned int ModuleFileSystem::Load(const char* path, const char* file, char** buffer) const
{
	std::string fullPath(path);
	fullPath += file;

	return Load(fullPath.data(), buffer);
}

// Read a whole file and put it in a new buffer
unsigned int ModuleFileSystem::Load(const char* file, char** buffer) const
{
	unsigned int ret = 0;

	PHYSFS_file* fs_file = PHYSFS_openRead(file);

	if(fs_file != nullptr)
	{
		PHYSFS_sint64 size = PHYSFS_fileLength(fs_file);

		if(size > 0)
		{
			*buffer = new char[(uint)size];
			PHYSFS_sint64 readed = PHYSFS_read(fs_file, *buffer, 1, (PHYSFS_sint32)size);
			if(readed != size)
			{
				LOG("File System error while reading from file %s: %s", file, PHYSFS_getLastError());
				if (buffer)
				{
					delete buffer;
				}
			}
			else
				ret = (uint)readed;
		}

		if(PHYSFS_close(fs_file) == 0)
			LOG("File System error while closing file %s: %s", file, PHYSFS_getLastError());
	}
	else
		LOG("File System error while opening file %s: %s", file, PHYSFS_getLastError());

	return ret;
}

// Read a whole file and put it in a new buffer
SDL_RWops* ModuleFileSystem::Load(const char* file) const
{
	char* buffer;
	int size = Load(file, &buffer);

	if(size > 0)
	{
		SDL_RWops* r = SDL_RWFromConstMem(buffer, size);
		if(r != nullptr)
			r->close = close_sdl_rwops;

		return r;
	}
	else
		return nullptr;
}


int close_sdl_rwops(SDL_RWops *rw)
{
	if (rw->hidden.mem.base)
	{
		delete[] rw->hidden.mem.base;
	}
	SDL_FreeRW(rw);
	return 0;
}

// Save a whole buffer to disk
unsigned int ModuleFileSystem::Save(const char* file, const char* buffer, unsigned int size) const
{
	unsigned int ret = 0;

	PHYSFS_file* fs_file = PHYSFS_openWrite(file);

	if(fs_file != nullptr)
	{
		PHYSFS_sint64 written = PHYSFS_write(fs_file, (const void*)buffer, 1, size);
		if (written != size)
		{
			LOG("File System error while writing to file %s: %s", file, PHYSFS_getLastError());
		}
		else
		{
			ret = (uint)written;
		}

		if(PHYSFS_close(fs_file) == 0)
			LOG("File System error while closing file %s: %s", file, PHYSFS_getLastError());
	}
	else
		LOG("File System error while opening file %s: %s", file, PHYSFS_getLastError());

	return ret;
}





// -----------------------------------------------------
// ASSIMP IO
// -----------------------------------------------------

size_t AssimpWrite(aiFile* file, const char* data, size_t size, size_t chunks)
{
	PHYSFS_sint64 ret = PHYSFS_write((PHYSFS_File*)file->UserData, (void*)data, size, chunks);
	if (ret == -1)
		LOG("File System error while WRITE via assimp: %s", PHYSFS_getLastError());

	return (size_t)ret;
}

size_t AssimpRead(aiFile* file, char* data, size_t size, size_t chunks)
{
	PHYSFS_sint64 ret = PHYSFS_read((PHYSFS_File*)file->UserData, (void*)data, size, chunks);
	if (ret == -1)
		LOG("File System error while READ via assimp: %s", PHYSFS_getLastError());

	return (size_t)ret;
}

size_t AssimpTell(aiFile* file)
{
	PHYSFS_sint64 ret = PHYSFS_tell((PHYSFS_File*)file->UserData);
	if (ret == -1)
		LOG("File System error while TELL via assimp: %s", PHYSFS_getLastError());

	return (size_t)ret;
}

size_t AssimpSize(aiFile* file)
{
	PHYSFS_sint64 ret = PHYSFS_fileLength((PHYSFS_File*)file->UserData);
	if (ret == -1)
		LOG("File System error while SIZE via assimp: %s", PHYSFS_getLastError());

	return (size_t)ret;
}

void AssimpFlush(aiFile* file)
{
	if (PHYSFS_flush((PHYSFS_File*)file->UserData) == 0)
		LOG("File System error while FLUSH via assimp: %s", PHYSFS_getLastError());
}

aiReturn AssimpSeek(aiFile* file, size_t pos, aiOrigin from)
{
	int res = 0;

	switch (from)
	{
	case aiOrigin_SET:
		res = PHYSFS_seek((PHYSFS_File*)file->UserData, pos);
		break;
	case aiOrigin_CUR:
		res = PHYSFS_seek((PHYSFS_File*)file->UserData, PHYSFS_tell((PHYSFS_File*)file->UserData) + pos);
		break;
	case aiOrigin_END:
		res = PHYSFS_seek((PHYSFS_File*)file->UserData, PHYSFS_fileLength((PHYSFS_File*)file->UserData) + pos);
		break;
	}

	if (res == 0)
		LOG("File System error while SEEK via assimp: %s", PHYSFS_getLastError());

	return (res != 0) ? aiReturn_SUCCESS : aiReturn_FAILURE;
}

aiFile* AssimpOpen(aiFileIO* io, const char* name, const char* format)
{
	static aiFile file;

	file.UserData = (char*)PHYSFS_openRead(name);
	file.ReadProc = AssimpRead;
	file.WriteProc = AssimpWrite;
	file.TellProc = AssimpTell;
	file.FileSizeProc = AssimpSize;
	file.FlushProc = AssimpFlush;
	file.SeekProc = AssimpSeek;

	return &file;
}

void AssimpClose(aiFileIO* io, aiFile* file)
{
	if (PHYSFS_close((PHYSFS_File*)file->UserData) == 0)
		LOG("File System error while CLOSE via assimp: %s", PHYSFS_getLastError());
}

std::string ModuleFileSystem::GetFileFormat(char* fullPath)
{
	char* tmp = fullPath;
	int size = 0;
	while (*tmp != '\0')
	{
		tmp++;
		size++;
	}
	while (*tmp != '.')
	{
		tmp--;
		size--;
		if (size <= 0)
		{
			return std::string("");
		}
	}
	tmp++;
	return std::string(tmp);
}

std::string ModuleFileSystem::RemoveFilePath(char * fileWithPath)
{
	char* it = fileWithPath;
	while (*it != '\0')
	{
		it++;
	}
	while (*it != '/'&& *it != '\\')
	{
		it--;
	}
	it++;
	return std::string(it);
}

void ModuleFileSystem::CreateAssimpIO()
{
	if (AssimpIO)
	{
		delete AssimpIO;
	}

	AssimpIO = new aiFileIO;
	AssimpIO->OpenProc = AssimpOpen;
	AssimpIO->CloseProc = AssimpClose;
}

aiFileIO * ModuleFileSystem::GetAssimpIO()
{
	return AssimpIO;
}

