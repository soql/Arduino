#include "VectorCollector.hpp"

using namespace AJSP;

VectorCollector::VectorCollector(Predicate pred):
		_predicate(pred)
{  
    _parser.setListener(this);
}

VectorCollector::~VectorCollector()
{
}

void VectorCollector::reset()
{
    _values.clear();
    _parser.reset();
}
     
void VectorCollector::value(const std::string& value, Parser::Entity entity)
{
	const std::string& path = _parser.getCurrentPath();
	if (!_predicate(path))
		return;

	_values.emplace_back(path, value, entity);
}

    
