#pragma once
#include <vector>
#include <memory>
#include <boost/serialization/version.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>

class TTTSerializer {
public:
	TTTSerializer(std::size_t hashCode, 
		std::shared_ptr<std::vector<unsigned>> buffer);
	template<class Archive> void serialize(Archive& ar, const unsigned int version);
	friend class boost::serialization::access;

private:
	std::size_t _hashCode;
	std::shared_ptr<std::vector<unsigned>>  _buffer;
};

template<class Archive>
void TTTSerializer::serialize(Archive& ar, const unsigned int version) {
	ar & _hashCode;
	ar & *_buffer;
}
