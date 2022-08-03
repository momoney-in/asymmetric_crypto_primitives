#define _CRT_SECURE_NO_WARNINGS
#include "asymmetric_crypto_primitives_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <optional>
#include <sstream>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

using namespace std;
using flutter::EncodableMap;
using flutter::EncodableValue;

namespace asymmetric_crypto_primitives {

// static
void AsymmetricCryptoPrimitivesPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "asymmetric_crypto_primitives",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<AsymmetricCryptoPrimitivesPlugin>();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

// Returns the data argument from |method_call| if it is present, otherwise
// returns an empty string.
std::string GetDataArgument(const flutter::MethodCall<>& method_call) {
  std::string data;
  const auto* arguments = std::get_if<EncodableMap>(method_call.arguments());
  if (arguments) {
    auto data_it = arguments->find(EncodableValue("data"));
    if (data_it != arguments->end()) {
      data = std::get<std::string>(data_it->second);
    }
  }
  return data;
}

// Returns the key argument from |method_call| if it is present, otherwise
// returns an empty string.
std::string GetKeyArgument(const flutter::MethodCall<>& method_call) {
  std::string key;
  const auto* arguments = std::get_if<EncodableMap>(method_call.arguments());
  if (arguments) {
    auto key_it = arguments->find(EncodableValue("key"));
    if (key_it != arguments->end()) {
      key = std::get<std::string>(key_it->second);
    }
  }
  return key;
}

void write_data(char* key,const char* data){
	std::ofstream outfile;
	char* appdata = getenv("APPDATA"); //get the path to folder Roaming AppData
	char* fileName = (char*) "\\passFile.txt"; //get the name of the .txt file
	
	//connect the path and the .txt file
	char * qq = (char*) malloc((strlen(appdata)+ strlen(fileName))*sizeof(char));
	strcpy(qq,appdata);
	strcat(qq,fileName);

	//open the file to write data to it
	outfile.open(qq, std::ios_base::app);
	
	//connect the key, delimiter : and data into one string
	char* data_to_write = (char*) malloc((strlen(key)+strlen(":")+ strlen(data))*sizeof(char));
	strcpy(data_to_write, key);
	strcat(data_to_write, ":");
	strcat(data_to_write, data);

	//write to file
	if(outfile.is_open()){
		outfile << data_to_write <<  std::endl;
	}
	
	//close the file
	outfile.close();
}

bool is_file_exist(const char *fileName){
    std::ifstream infile(fileName);
    return infile.good();
}

std::string read_data(char* key){
	char* appdata = getenv("APPDATA"); //get the path to folder Roaming AppData
	char* fileName = (char*) "\\passFile.txt"; //get the name of the .txt file

	//connect the path and the .txt file
	char * qq = (char*) malloc((strlen(appdata)+ strlen(fileName))*sizeof(char));
	strcpy(qq,appdata);
	strcat(qq,fileName);

	//check if the file exists to avoid reading data before it is written
	if(is_file_exist(qq)){
		std::ifstream file(qq);
		if (file.is_open()) {
			std::string line;
			//get each line of the .txt file
			while (std::getline(file, line)) {
				//create a string from each line
				std::string myLine(line.c_str());
				//if line contains the key (here uuid), return the data (here, key to sign)
				if (myLine.find(key) != std::string::npos) {
					return myLine.substr(myLine.find(":")+1);
				}
			}
		//close the file
		file.close();
		}
		//if something goes wrong, return empty string
		return "";
	}else{
		//if something goes wrong, return empty string
		return "";
	}
}

void delete_data(char* key){
	char* appdata = getenv("APPDATA"); //get the path to folder Roaming AppData
	char* fileName = (char*) "\\passFile.txt"; //get the name of the .txt file
	char* fileNameTemp = (char*) "\\passFileTemp.txt"; //get the name of the temporary .txt file
	
	//connect the path and the .txt file
	char * qq = (char*) malloc((strlen(appdata)+ strlen(fileName))*sizeof(char));
	strcpy(qq,appdata);
	strcat(qq,fileName);
	
	//connect the path and the temporary .txt file
	char * qq2 = (char*) malloc((strlen(appdata)+ strlen(fileNameTemp))*sizeof(char));
	strcpy(qq2,appdata);
	strcat(qq2,fileNameTemp);
	
	//create streams to read from .txt and write to temp .txt
	std::ifstream file_to_read(qq);
	std::ofstream file_to_write(qq2);
	
	//while the .txt file has not reached its end
	if (!file_to_read.eof()) {
		std::string line;
		//get each line of the .txt file
		while (std::getline(file_to_read, line)) {
			//create a string from each line
	    	std::string myLine(line.c_str());
			//if line contains the key (here uuid), do nothing
	    	if (myLine.find(key) != std::string::npos) {
			}else{
				//else write the data to the temporary file
				file_to_write << myLine << std::endl;
			}
		}
	}
	
	//close both files
	file_to_read.close();
	file_to_write.close();
	//remove the original .txt file and rename the temp file to the original
	remove(qq);
	rename(qq2, qq);
}

void update_data(char* key, const char* data){
	char* appdata = getenv("APPDATA"); //get the path to folder Roaming AppData
	char* fileName = (char*) "\\passFile.txt"; //get the name of the .txt file
	char* fileNameTemp = (char*) "\\passFileTemp.txt"; //get the name of the temporary .txt file
	
	//connect the path and the .txt file
	char * qq = (char*) malloc((strlen(appdata)+ strlen(fileName))*sizeof(char));
	strcpy(qq,appdata);
	strcat(qq,fileName);
	
	//connect the path and the temporary .txt file
	char * qq2 = (char*) malloc((strlen(appdata)+ strlen(fileNameTemp))*sizeof(char));
	strcpy(qq2,appdata);
	strcat(qq2,fileNameTemp);
	
	//create streams to read from .txt and write to temp .txt
	std::ifstream file_to_read(qq);
	std::ofstream file_to_write(qq2);
	
	//while the .txt file has not reached its end
	if (!file_to_read.eof()) {
		std::string line;
		//get each line of the .txt file
		while (std::getline(file_to_read, line)) {
			//create a string from each line
	    	std::string myLine(line.c_str());
			//if line contains the key (here uuid)
	    	if (myLine.find(key) != std::string::npos) {
				//connect the uuid, delimiter and new data into one string
				char* data_to_write = (char*) malloc((strlen(key)+strlen(":")+ strlen(data))*sizeof(char));
				strcpy(data_to_write, key);
				strcat(data_to_write, ":");
				strcat(data_to_write, data);
				//write the new line to the temp file
				file_to_write << data_to_write << std::endl;
			}else{
				//else just write the original line to the temp file
				file_to_write << myLine << std::endl;
			}
		}
	}
	//close both files 
	file_to_read.close();
	file_to_write.close();
	//remove the original .txt file and rename the temp file to the original
	remove(qq);
	rename(qq2, qq);
}


AsymmetricCryptoPrimitivesPlugin::AsymmetricCryptoPrimitivesPlugin() {}

AsymmetricCryptoPrimitivesPlugin::~AsymmetricCryptoPrimitivesPlugin() {}

void AsymmetricCryptoPrimitivesPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    cout << method_call.method_name() << endl;
  if (method_call.method_name().compare("getPlatformVersion") == 0) {
    std::ostringstream version_stream;
    version_stream << "Windows ";
    if (IsWindows10OrGreater()) {
      version_stream << "10+";
    } else if (IsWindows8OrGreater()) {
      version_stream << "8";
    } else if (IsWindows7OrGreater()) {
      version_stream << "7";
    }
    result->Success(flutter::EncodableValue(version_stream.str()));
  }else if(method_call.method_name().compare("writeData") == 0){
    std::string data = GetDataArgument(method_call);
    std::string key = GetKeyArgument(method_call);
    write_data((char*) key.c_str(), data.c_str());
    result->Success(flutter::EncodableValue(true));
  }else if(method_call.method_name().compare("readData") == 0){
    std::string key = GetKeyArgument(method_call);
    std::string data = read_data((char*) key.c_str());
    if(data.empty()){
      result->Success(flutter::EncodableValue(false));
    }else{
      result->Success(flutter::EncodableValue(data));
    }
  }else if(method_call.method_name().compare("deleteData") == 0){
    std::string key = GetKeyArgument(method_call);
    delete_data((char*) key.c_str());
    result->Success(flutter::EncodableValue(true));
  }else if(method_call.method_name().compare("editData") == 0){
    std::string data = GetDataArgument(method_call);
    std::string key = GetKeyArgument(method_call);
    update_data((char*) key.c_str(), data.c_str());
    result->Success(flutter::EncodableValue(true));
  }
  else {
    result->NotImplemented();
  }
}




}  // namespace asymmetric_crypto_primitives