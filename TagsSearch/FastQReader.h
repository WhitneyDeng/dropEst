#pragma once

#include <atomic>
#include <fstream>
#include <mutex>
#include <vector>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <Tools/ScSpConcurrentQueue.h>

#include "Tools/ReadParameters.h"

namespace TagsSearch
{
	class FastQReader
	{
	private:
		using mutex_t = std::mutex;

	public:
		struct FastQRecord
		{
			explicit FastQRecord(const std::string &id="", const std::string &sequence="",
			                     const std::string &description="", const std::string &quality="");

			std::string id;
			std::string sequence;
			std::string description;
			std::string quality;

			std::string to_string() const;
		};

	private:
		const std::string _filename;
		std::atomic<bool> _file_ended;
		Tools::ScSpConcurrentQueue<FastQRecord> _records;
		mutex_t _read_mutex;

		std::ifstream _in_file;
		boost::iostreams::filtering_istream _in_fstream;

	private:
		bool get_next_line_unsafe(std::string &line);

		/// Read next block from the fastq file and put it to record. Not thread-safe!
		///
		/// \param record fastq records where output should be placed
		/// \return true if read successful, false if EOF before the beginning of block
		bool get_next_record_unsafe(FastQRecord &record);

	public:
		explicit FastQReader(const std::string &filename);

		const std::string& filename() const;

		/// Try to read next set of records for the fastq file to cache until it's full.
		/// Do nothing if another thread is already reading this file or if its end is reached.
		void try_read_records_to_cash();

		bool get_next_record(FastQRecord &record);
	};
}
