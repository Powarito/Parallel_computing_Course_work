# Parallel computing course work

This repository contains code and UML-diagrams for the parallel computing course work.

Server directory contains the C++ project for the server, Client directory contains the C++ project for the client with admin rights. Stress_Test_Client contains the C++ project for stress-testing the server.
Client_Python contains the Python project for the client with read-only rights.

Shared_Files directory simply contains the C++ header files for both the code from the Server and Client directories.

# Build instructions
For C++ Server, Client and Stress_Test_Client:
1. Download and install Visual Studio with support for C++23.
2. Download and extract the repository to a convenient location for you.
3. Navigate to Parallel_computing_Course_work directory.
4. Open Parallel_computing_Course_work.sln in Visual Studio.
5. On top, select Debug and change the build configuration to Release for better performance.
6. Go to Project -> Properties -> Configuration Properties -> General. Make sure that the C++ Language Standard field is set to ISO C++23 Standard (C++latest).
7. Go to Project -> Properties -> Configuration Properties -> VC++ Directories. Make sure that in 'Include Directories' input field there's '<your_repo_location>/Parallel_computing_Course_work/Shared_files;' entry, where <your_repo_location> is the location where you downloaded and extracted the repository.
8. Click Build -> Build Solution
9. Go back to the folder with the solution.
10. Navigate to \x64\Release.
11. Launch the .exe file.

For Python Client:
1. Install Python.
* Download and install Python from https://www.python.org/.
* During installation, make sure to check the option Add Python to PATH.
2. Install Visual Studio Code
* Download and install Visual Studio Code from https://code.visualstudio.com/.
3. Install the Python Extension for Visual Studio Code
* Open Visual Studio Code.
* Click on the Extensions icon on the left (or press Ctrl+Shift+X).
* Search for Python and install the extension by Microsoft.
4. Open the Project
* Download and extract the repository.
* In Visual Studio Code, go to File -> Open Folder... and select the folder containing your Python code.
5. Select the Python Interpreter
* Press Ctrl+Shift+P (or F1) and search for Python: Select Interpreter.
* Choose your installed version of Python.
6. Run the Code
* Open the desired Python script in Visual Studio Code.
* Click the Run button in the top-right corner or press F5.
