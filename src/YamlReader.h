#pragma once
#include <cstdlib>
#include <string>
#include <vector>
#include <exception>
#include "types.h"
#include "libyaml/yaml.h"

/*
class YamlException : public std::exception
{
public:
	YamlException(const std::string& error) throw()
	{
		what_ = error;
	}

	~YamlException() throw()
	{

	}

	virtual const char* what() const throw()
	{
		return what_.c_str();
	}
private:
	std::string what_;
};
*/

class YamlReader
{
public:
	YamlReader();
	~YamlReader();

	int LoadFile(const char* path);

	// returns a reference to the current event string
	inline const std::string& event_string(void) const { return event_str_; }

	// copies the key's value (or sequence of values) to a referenced dst
	int SaveValue(std::string& dst);
	int SaveValueSequence(std::vector<std::string>& dst);

	// yaml event controls
	bool GetEvent();
	inline u32 level() const { return level_; }
	inline bool is_level_in_scope(u32 level) const { return level_ >= level; }
	inline bool is_level_same(u32 level) const { return level_ == level; }
	inline bool is_done() const { return is_done_; }
	inline bool is_error() const { return is_api_error_; }

	inline bool is_event_nothing() const { return event_.type == YAML_NO_EVENT; }
	inline bool is_event_scalar() const { return event_.type == YAML_SCALAR_EVENT; }
	inline bool is_event_mapping_start() const { return event_.type == YAML_MAPPING_START_EVENT; }
	inline bool is_event_mapping_end() const { return event_.type == YAML_MAPPING_END_EVENT; }
	inline bool is_event_sequence_start() const { return event_.type == YAML_SEQUENCE_START_EVENT; }
	inline bool is_event_sequence_end() const { return event_.type == YAML_SEQUENCE_END_EVENT; }

	inline bool is_sequence() const { return is_sequence_; }
	inline bool is_key() const { return is_key_; }

private:
	// for libyaml
	FILE *yaml_file_ptr_;
	yaml_parser_t parser_;
	yaml_event_t event_;
	bool is_done_;
	bool is_api_error_;

	// for event control
	bool is_sequence_;
	bool is_key_;
	u32 level_;

	std::string event_str_;

	void Cleanup();
};