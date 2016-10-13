
# V_Engine

A simple game engine developed by Vulpem.     
This has been created for learning purposes, during 2016 at the CITM university.   

Source code can be found at:        
https://github.com/Vulpem/V_Engine/

## Usage

The editor will import all the 3d scenes and textures located inside of the "Assets" folder at the beggining of execution.     
Any changes done to those files during execution or files from outside this folder won't be handled by the engine.         
Note that importing all files may take some time. The application may take a few seconds before initializing.

Drop any 3dFile into the scene to load it.    
Alternatively, you can use the "Import Geometry" menu. To do so, just input the geometry you want to import.


All tmp files are created in "%appdata%/UPC/V_Engine/Library"      
Note: All imported files will be converted into a ".vmesh". Avoid having multiple files with the same name, even if they're     
in different folders or different formats, since it may cause one of the ".vmesh"es to be overwritten.   


## LICENSE

 Copyright 2016 David Hernandez

   Licensed under the Apache License, Version 2.0 (the "License");    
   you may not use this file except in compliance with the License.    
   You may obtain a copy of the License at    

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software     
   distributed under the License is distributed on an "AS IS" BASIS,    
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.    
   See the License for the specific language governing permissions and    
   limitations under the License.