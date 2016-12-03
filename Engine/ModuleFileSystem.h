#ifndef __j1FILESYSTEM_H__
#define __j1FILESYSTEM_H__

#include "Module.h"
#include <vector>

struct SDL_RWops;
int close_sdl_rwops(SDL_RWops *rw);

struct aiFileIO;

struct Date
{
	uint year = 0;
	uint month = 0;
	uint day = 0;
	uint hour = 0;
	uint min = 0;
	uint sec = 0;

	bool operator ==(const Date b)
	{
		if (year == b.year && month == b.month && day == b.day && hour == b.hour && min == b.min && sec == b.sec)
		{
			return true;
		}
		return false;
	}

	bool operator !=(const Date b)
	{
		return !(*this == b);
	}

};

class ModuleFileSystem : public Module
{
public:

	ModuleFileSystem(Application* app, bool start_enabled = true);

	// Destructor
	virtual ~ModuleFileSystem();

	// Called before render is available
	//bool Awake(pugi::xml_node&);
	bool Init();

	// Called before quitting
	bool CleanUp();

	// Utility functions
	bool AddPath(const char* path_or_zip, const char* mount_point = nullptr);
	bool Exists(const char* file) const;
	bool CreateDir(const char* dir);
	bool DelDir(const char* dir);
	bool DelFile(const char* file);
	bool IsDirectory(const char* file) const;
	std::string GetWrittingDirectory();
	void GetFilesIn(const char * directory, std::vector<std::string>* folders, std::vector<std::string>* files);

	Date ReadFileDate(const char* path);

	bool EraseFile(const char* file);

	// Open for Read/Write
	unsigned int Load(const char* path, const char* file, char** buffer) const;
	unsigned int Load(const char* file, char** buffer) const;
	SDL_RWops* Load(const char* file) const;

	// IO interfaces for other libs to handle files via PHYSfs
	aiFileIO* GetAssimpIO();

	unsigned int Save(const char* file, const char* buffer, unsigned int size) const;

	const char* GetSaveDirectory() const
	{
		return "save/";
	}

	std::string GetFileFormat(char* fullPath);
	std::string RemoveFilePath(char* fileWithPath);

private:

	void CreateAssimpIO();

private:

	aiFileIO* AssimpIO = nullptr;
};

#endif // __j1FILESYSTEM_H__