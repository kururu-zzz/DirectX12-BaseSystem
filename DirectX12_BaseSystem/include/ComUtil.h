#pragma once
#include <objbase.h>

class ComApartment{
public:
	ComApartment() {
		::CoInitialize(NULL);
	}
	~ComApartment() {
		::CoUninitialize();
	}
};