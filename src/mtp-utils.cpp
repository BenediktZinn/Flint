//
// Created by Benedikt Zinn on 29.12.24.
//

#include <libmtp.h>
#include <iostream>
#include <vector>
#include <string>

// Function to initialize libmtp and find connected devices
LibMTP_Device *findMTPDevice() {
    // Initialize libmtp
    LibMTP_Init();

    // Get the number of devices connected
    int numDevices = LibMTP_Get_Count();

    if (numDevices == 0) {
        std::cerr << "No MTP devices found." << std::endl;
        return nullptr;
    }

    // Get the first connected MTP device
    LibMTP_Device *mtpDevice = LibMTP_Get_Device(0);

    if (!mtpDevice) {
        std::cerr << "Failed to get the MTP device." << std::endl;
        return nullptr;
    }

    std::cout << "MTP device found: " << LibMTP_Get_Model(mtpDevice) << std::endl;
    return mtpDevice;
}

// Function to browse files and directories on the MTP device
void browseFiles(LibMTP_Device *mtpDevice) {
    if (!mtpDevice) {
        std::cerr << "Invalid MTP device." << std::endl;
        return;
    }

    // List all files in the root directory of the MTP device
    LibMTP_File *files = LibMTP_Get_Filelisting(mtpDevice, 0);  // 0 means root directory
    if (!files) {
        std::cerr << "Failed to get file listing from the device." << std::endl;
        return;
    }

    std::cout << "Listing files in the root directory:" << std::endl;
    LibMTP_File *file = files;
    while (file) {
        std::cout << "File: " << file->filename << " (Type: " << file->filetype << ")" << std::endl;
        file = file->next;
    }

    // Free the memory used by the file list
    LibMTP_Release_Filelisting(files);
}

// Function to download a file from the MTP device to the local machine
bool downloadFile(LibMTP_Device *mtpDevice, const std::string &filename, const std::string &localPath) {
    if (!mtpDevice) {
        std::cerr << "Invalid MTP device." << std::endl;
        return false;
    }

    // Find the file by name (in the root directory for simplicity)
    LibMTP_File *files = LibMTP_Get_Filelisting(mtpDevice, 0);
    if (!files) {
        std::cerr << "Failed to get file listing from the device." << std::endl;
        return false;
    }

    LibMTP_File *file = files;
    LibMTP_File *fileToDownload = nullptr;

    while (file) {
        if (filename == file->filename) {
            fileToDownload = file;
            break;
        }
        file = file->next;
    }

    if (!fileToDownload) {
        std::cerr << "File not found on device." << std::endl;
        LibMTP_Release_Filelisting(files);
        return false;
    }

    // Download the file
    std::cout << "Downloading file: " << filename << std::endl;
    FILE *outputFile = fopen(localPath.c_str(), "wb");
    if (!outputFile) {
        std::cerr << "Failed to open file for writing: " << localPath << std::endl;
        LibMTP_Release_Filelisting(files);
        return false;
    }

    if (LibMTP_Get_File_To_File(mtpDevice, fileToDownload->id, outputFile) != 0) {
        std::cerr << "Failed to download file." << std::endl;
        fclose(outputFile);
        LibMTP_Release_Filelisting(files);
        return false;
    }

    std::cout << "File downloaded successfully!" << std::endl;
    fclose(outputFile);
    LibMTP_Release_Filelisting(files);
    return true;
}

// Function to upload a file from the local machine to the MTP device
bool uploadFile(LibMTP_Device *mtpDevice, const std::string &localPath, const std::string &devicePath) {
    if (!mtpDevice) {
        std::cerr << "Invalid MTP device." << std::endl;
        return false;
    }

    // Open the file to be uploaded
    FILE *inputFile = fopen(localPath.c_str(), "rb");
    if (!inputFile) {
        std::cerr << "Failed to open file for reading: " << localPath << std::endl;
        return false;
    }

    // Upload the file
    std::cout << "Uploading file: " << localPath << " to " << devicePath << std::endl;
    if (LibMTP_Send_File_From_File(mtpDevice, inputFile, devicePath.c_str()) != 0) {
        std::cerr << "Failed to upload file." << std::endl;
        fclose(inputFile);
        return false;
    }

    std::cout << "File uploaded successfully!" << std::endl;
    fclose(inputFile);
    return true;
}

// Main function to test MTP communication
int main() {
    LibMTP_Device *mtpDevice = findMTPDevice();

    if (mtpDevice) {
        browseFiles(mtpDevice);  // Browse files on the MTP device
        // Example to download a file
        downloadFile(mtpDevice, "example.txt", "/path/to/local/example.txt");

        // Example to upload a file
        uploadFile(mtpDevice, "/path/to/local/upload.txt", "/path/to/device/upload.txt");

        // Release the MTP device after use
        LibMTP_Release_Device(mtpDevice);
    }

    // Shutdown libmtp
    LibMTP_Shutdown();
    return 0;
}
