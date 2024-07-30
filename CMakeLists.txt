cmake_minimum_required(VERSION 3.28)

include(Cango)

project("Cango.TaskDesign")

AddCXXModule(
	NAME TaskDesign
	NAMESPACE Cango
	LINKS
		"Cango_CommonUtils_IntervalSleeper"
		"Cango_CommonUtils_CounterX"
		"Boost::system"
)	
