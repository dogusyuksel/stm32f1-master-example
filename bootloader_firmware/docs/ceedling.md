
This is a small project with thirt party library and hw dependent library (both fake) and how to combine all the folders with ceedling by using cmock.

Some steps I found useful:
* goto parent folder of the project
* type "ceedling new <projectname>"
* navigate the project and execute "ceedling test:all"
* note that source codes must be in "src" folder
* You can confirm that Ceedling knows about your source files by running "ceedling files:source"
* ceedling module:create[<newmodulename>]
* ceedling test:all --> again to see errors
