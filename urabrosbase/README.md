# UrabrosBase
Framework created for STM32 MicroControllers.  
This framework is wrapper for FreeRTOS system, giving us a lot usefull features.  
The main idea behind it is that there is a driver PC or MC what can start stop resume delete Tasks. 
A task is responsible for a well described process. Forexample: An uTask for vending machine: Give me two minerall water bottles.  
The drier PC can get the actual status of the active tasks, so it can make decisions.  
This repo holds the common framework related files, the configuration of Urabros and adding actual tasks are in thhe sub repositories.  
At Templates directory you can find some empty task templates also some examples. Maybe the examples are a bit outdated, for better example search already implemented projects like the NPS V2: 

## LINKS
Documentation:     
Presentation:        
Tester Software:     
Teaching Material: 
Git Strategy:       
Communication Prot:  

## Documentation.
Documentation can be found here on gitlab, as a statically hosted webpage, it is already linked in section LINKS  
It is updated when something is pushed to the master branch.  
It is done by a pipeline, what's settings can be modified in the .gitlab-ci.yml file  
When you are in a FEATURE-Branch dont push the modifications what were made in the /public directory. Because it has over 300 files and it makes the history unuseable.  

### Install and usage of DoxyGen
Link for Install guide: https://www.doxygen.nl/manual/install.html Windows installation at the bottom.  
To generate documentation open Doxywizard, in the wizard: --> file-->open this file: .../urabrosframework/DoxyGenWorkingDir/DoxyUrabrosV2  
In theory all the paths are dynamic so nothing els should be done.  
At Run/Run doxygen wait its process, than press show HTML and the new documentation is generated, simple isnt it?
For doxygen syntax guide follow the syntax in other files.
