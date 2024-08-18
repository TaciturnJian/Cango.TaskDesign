cmake_minimum_required(VERSION 3.28)

include(Cango)

project("Cango.TaskDesign")

set(RequiredLibraries
	"Cango::CommonUtils"
)
RequireLibraries(RequiredLibraries)

AddCXXModule(
	NAME TaskDesign
	NAMESPACE Cango
	LINKS
		"Cango_CommonUtils::CounterX"
		"Cango_CommonUtils::IntervalSleeper"
		"Cango_CommonUtils::ObjectOwnership"
)	
