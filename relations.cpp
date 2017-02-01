#include "relations_builder.h"
#include "relations.h"

enum class SType {Assign, Compound, Branch, Loop};

template<>
class Statement<SType::Assign>
{
public:
};

template<>
class Statement<SType::Compound>
{
public:
};

template<>
class Statement<SType::Branch>
{
public:
};

template<>
class Statement<SType::Loop>
{
public:
};
