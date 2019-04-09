#pragma once

#include "Counters/MultiSpacerOutcomesCounter.h"
#include "SpacerFinder.h"
#include "TagsFinderBase.h"

namespace TestTagsSearch
{
	struct testMask;
	struct testMaskParse;
}

namespace TagsSearch
{
	class FixPosSpacerTagsFinder : public TagsFinderBase
	{
		friend struct TestTagsSearch::testMask;
		friend struct TestTagsSearch::testMaskParse;

	protected:
		struct MaskPart
		{
			enum Type
			{
				SPACER,
				CB,
				UMI,
				NONE
			};
			std::string spacer;
			size_t length;
			Type type;
			size_t min_edit_distance;

			explicit MaskPart(const std::string &spacer="", size_t length=0, Type type=Type::NONE, size_t min_edit_distance=0);
		};

	private:
		const std::vector<MaskPart> _mask_parts;
		const size_t _trim_tail_length;

		MultiSpacerOutcomesCounter _outcomes;

	private:
		size_t parse(const std::string &r1_seq, const std::string &r1_quality, Tools::ReadParameters &read_params);

		static std::vector<MaskPart> parse_mask(const std::string& barcode_mask, const std::string& edit_dist_str);
		static size_t parse_barcode_mask(const std::string &mask, size_t cur_pos, MaskPart &mask_part);

	protected:
		bool parse_fastq_records(FastQReader::FastQRecord &gene_record, Tools::ReadParameters &read_params) override;

		std::string get_additional_stat(long total_reads_read) const override;

	public:
		FixPosSpacerTagsFinder(const std::vector<std::string> &fastq_filenames,
		                       const boost::property_tree::ptree &barcodes_config,
		                       const boost::property_tree::ptree &trimming_config,
		                       const std::shared_ptr<ConcurrentGzWriter> &writer,
		                       bool save_stats, bool save_read_params);
	};
}
