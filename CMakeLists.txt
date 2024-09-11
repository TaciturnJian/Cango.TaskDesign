cmake_minimum_required(VERSION 3.28)

include(Cango)

project("Cango.TaskDesign")

set(required_libraries
	"Cango::CommonUtils"
)
RequireLibraries(required_libraries)

AddCXXModule(
	NAME TaskDesign
	NAMESPACE Cango
	LINKS
		"Cango::CommonUtils"
		"matchit::matchit"
)	
