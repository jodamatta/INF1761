#pragma once
#include <memory>
class Disk;
using DiskPtr = std::shared_ptr<Disk>;

#ifndef Disk_H
#define Disk_H

#include "shape.h"

class Disk : public Shape {
	unsigned int m_vao;
protected:
	Disk(int numVertex);
public:
	static DiskPtr Make(int numVertex);
	virtual ~Disk();
	virtual void Draw(StatePtr st);
};
#endif