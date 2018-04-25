#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include "Libraries\dirent\dirent.h"
#include "Libraries\rapidjson\document.h"

using namespace std;

void replace(string& src, string const& find, string const& replace)
{
    for(string::size_type i = 0; (i = src.find(find, i)) != string::npos;)
    {
        src.replace(i, find.length(), replace);
        i += replace.length();
    }
}

bool dirExists(const std::string& dirName_in)
{
	 DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
	 if (ftyp == INVALID_FILE_ATTRIBUTES)
		  return false;  //something is wrong with your path!
	 if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		  return true;   // this is a directory!
	 
	 return false;    // this is not a directory!
}

std::vector<std::string> getFilesInDir(const char* dirName) {
	 std::vector<std::string> files;
	 DIR* dir;
	 struct dirent* ent;

	 if((dir = opendir(dirName)) != NULL) {
		  while((ent = readdir(dir)) != NULL) {
				if (strcmp(ent->d_name, ".") != 0 &&
					 strcmp(ent->d_name, "..") != 0 ) {
					 files.push_back(ent->d_name);
				}				
		  }
		  closedir(dir);
	 }

	 return files;
}


int main(int argc, char* argv[])
{
	 if (argc < 2) {
		  cout << "Expected at least 2 arguments!" << endl;
		  cin.get();
		  return EXIT_FAILURE;
	 }

	 if (strcmp(argv[1], "setup") == 0) {
		  cout << "Setting up workspace..." << endl;
		  CreateDirectory("sites", NULL);
		  CreateDirectory("templates", NULL);
	 }

	 if (strcmp(argv[1], "tie") == 0) {
		  string dirOutput = "public";

		  if(!dirExists(dirOutput)) {
				cout << "Creating directory " << dirOutput << "...";
				CreateDirectory(dirOutput.c_str(), NULL);
		  }

		  vector<string> filesTemplates = getFilesInDir("templates");
		  cout << filesTemplates.size() << " template(s) found" << endl;

		  vector<string> filesSites = getFilesInDir("sites");
		  cout << filesSites.size() << " site(s) found" << endl;

		  for(std::vector<string>::size_type i = 0; i != filesSites.size(); i++) {
				string tmpFilePath;
				tmpFilePath.append("sites/");
				tmpFilePath.append(filesSites[i]);

				ifstream tmpFile(tmpFilePath);
				string tmpFileResult;

				if(!tmpFile) {
					 cerr << "Unable to open file";
					 return EXIT_FAILURE;
				}

				for(string line; getline(tmpFile, line);)
				{
					 tmpFileResult.append(line);
				}

				const char* tmpFileResultC = tmpFileResult.c_str();

				rapidjson::Document jsond;
				jsond.Parse(tmpFileResultC);

				rapidjson::Value& configName = jsond["name"];
				rapidjson::Value& configTemplate = jsond["template"];
				rapidjson::Value& configTitle = jsond["title"];
				rapidjson::Value& configBody = jsond["body"];
				rapidjson::Value& configStyles = jsond["styles"];
				rapidjson::Value& configScripts = jsond["scripts"];

				string tmpTemplatePath;
				tmpTemplatePath.append("templates/");
				tmpTemplatePath.append(configTemplate.GetString());
				tmpTemplatePath.append(".html");

				ifstream tmpTemplate(tmpTemplatePath);
				string tmpTemplateResult;

				if(!tmpTemplate) {
					 cerr << "Unable to open file";
					 return EXIT_FAILURE;
				}

				for(string line; getline(tmpTemplate, line);)
				{
					 tmpTemplateResult.append(line);
				}

				string tmpResultPath;
				tmpResultPath.append("public/");
				tmpResultPath.append(configName.GetString());
				tmpResultPath.append(".html");

				ofstream tmpResult(tmpResultPath);

				replace(tmpTemplateResult, "$title", configTitle.GetString());
				replace(tmpTemplateResult, "$body", configBody.GetString());
				replace(tmpTemplateResult, "$styles", configStyles.GetString());
				replace(tmpTemplateResult, "$scripts", configScripts.GetString());

				tmpResult << tmpTemplateResult;

				tmpFile.close();
				tmpTemplate.close();
				tmpResult.close();
		  }
	 }

	 cin.get();
    return EXIT_SUCCESS;
}