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
	HEADER_DIR Headers
	SOURCE_DIR Sources
	TESTER_DIR Testers
	LINKS
		"Cango::CommonUtils"
#		"matchit::matchit"
)	
