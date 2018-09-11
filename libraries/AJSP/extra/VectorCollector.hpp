#ifndef VECTORCOLLECTOR_HPP
#define VECTORCOLLECTOR_HPP

#include "AJSP.hpp"
#include "PathConstructor.hpp"
#include <functional>

struct KeyValueType
{
    KeyValueType(const std::string& key, const std::string& value, AJSP::Parser::Entity entity):
        key(key), value(value), entity(entity) {}

    std::string key;
    std::string value;
    AJSP::Parser::Entity entity;
};

template <class T>
bool True(const T& t)
{
	return true;
}

class VectorCollector: private AJSP::Listener
{
    public:
		using Predicate = std::function<bool(const std::string& path)>;

        VectorCollector(Predicate p = True<std::string>);
        ~VectorCollector();

        void reset();
        AJSP::Parser::Result parse(char c) {return _parser.parse(c);}
        bool done() const {return _parser.done();}

        const std::vector<KeyValueType>& getValues() const {return _values;}

    private:
        virtual void value(const std::string& value, AJSP::Parser::Entity entity);

        AJSP::Parser                _parser;
        std::vector<KeyValueType>   _values;
        Predicate					_predicate;
};

#endif //VECTORCOLLECTOR_HPP
