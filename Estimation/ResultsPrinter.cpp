#include "ResultsPrinter.h"

#include <RInside.h>

#include <Estimation/CellsDataContainer.h>
#include <Tools/Logs.h>
#include <Tools/UtilFunctions.h>

using namespace Rcpp;

namespace Estimation
{
	const size_t ResultsPrinter::top_print_size;

	void ResultsPrinter::save_results(const CellsDataContainer &container, const std::string &filename) const
	{
		if (container.filtered_cells().empty())
		{
			L_WARN << "WARNING: filtered cells are empty. Probably, filtration threshold is too strict"
			       << " or you forgot to run 'merge_and_filter'";
		}
		RInside *R = Tools::init_r();

		this->trace_gene_counts(container);

		L_TRACE << "Compiling diagnostic stats: ";
		auto reads_per_chr_per_cell = this->get_reads_per_chr_per_cell_info(container); // Real cells, all UMIs.
		auto saturation_info = this->get_saturation_analysis_info(container); // Filtered cells, query UMIs.
		auto mean_reads_per_umi = this->get_mean_reads_per_umi(container); // Real cells, all UMIs.
		auto reads_per_umi_per_cell = this->get_reads_per_umi_per_cell(container); // Filtered cells, query UMIs.
		auto merge_targets = this->get_merge_targets(container); // All cells.
		IntegerVector aligned_reads_per_cb = wrap(container.get_stat_by_real_cells(Stats::TOTAL_READS_PER_CB)); // Real cells, all UMIs
		IntegerVector aligned_umis_per_cb = wrap(container.get_stat_by_real_cells(Stats::TOTAL_UMIS_PER_CB)); // Real cells, all UMIs
		auto requested_umis_per_cb = this->get_requested_umis_per_cb(container); // Real cells, query UMIs
		auto requested_reads_per_cb = this->get_requested_umis_per_cb(container, true); // Real cells, query UMIs
		L_TRACE << "Completed.\n";

		(*R)["d"] = List::create(
				_["cm"] = this->get_count_matrix(container, true),
				_["cm_raw"] = this->get_count_matrix(container, false),
				_["reads_per_chr_per_cells"] = reads_per_chr_per_cell,
				_["reads_per_umi_per_cell"] = reads_per_umi_per_cell,
				_["mean_reads_per_umi"] = mean_reads_per_umi,
				_["saturation_info"] = saturation_info,
				_["merge_targets"] = merge_targets, // TODO: optimize it
				_["aligned_reads_per_cell"] = aligned_reads_per_cb,
				_["aligned_umis_per_cell"] = aligned_umis_per_cb,
				_["requested_umis_per_cb"] = requested_umis_per_cb,
				_["requested_reads_per_cb"] = requested_reads_per_cb);

		std::string filename_base = filename;
		auto extension_pos = filename.find_last_of(".");
		if (extension_pos != std::string::npos && filename.substr(extension_pos + 1) == "rds")
		{
			filename_base = filename.substr(0, extension_pos);
		}

		std::string rds_filename = filename_base + ".rds";

		L_TRACE << "";
		Tools::trace_time("Writing R data to " + rds_filename + " ...");
		R->parseEvalQ("saveRDS(d, '" + rds_filename + "')");
		Tools::trace_time("Completed");

		if (this->write_matrix) {
			std::string mtx_file = filename_base + ".mtx";
			L_TRACE << "Writing " + mtx_file + " ...";
			R->parseEvalQ("Matrix::writeMM(d$cm, '" + mtx_file + "')");
			R->parseEvalQ("write.table(colnames(d$cm), '" + filename_base + ".cells.tsv', row.names = F, col.names = F, quote = F)");
			R->parseEvalQ("write.table(rownames(d$cm), '" + filename_base + ".genes.tsv', row.names = F, col.names = F, quote = F)");
			L_TRACE << "Completed.";
		}

		L_TRACE << "";
	}

	IntegerMatrix ResultsPrinter::create_matrix(const s_vec_t &col_names, const s_vec_t &row_names,
	                            const i_vec_t &counts)
	{
		IntegerMatrix mat(transpose(IntegerMatrix(int(col_names.size()), int(row_names.size()), counts.begin())));

		colnames(mat) = wrap(col_names);
		rownames(mat) = wrap(row_names);

		return mat;
	}

	ResultsPrinter::ResultsPrinter(bool write_matrix, bool reads_output)
		: write_matrix(write_matrix)
		, reads_output(reads_output)
	{}

	List ResultsPrinter::get_saturation_analysis_info(const CellsDataContainer &container) const
	{
		L_TRACE << "Saturation info;";
		i_vec_t reads_by_umig;
		s_vec_t reads_by_umig_cbs;
		s_vec_t reads_by_umig_umis;

		for (size_t cell_id = 0; cell_id < container.total_cells_number(); ++cell_id)
		{
			auto const &cell = container.cell(cell_id);
			if (!cell.is_real())
				continue;

			const auto &cb = cell.barcode();
			for (auto const& gene : cell.requested_reads_per_umi_per_gene())
			{
				for (auto const &reads_per_umi : gene.second)
				{
					reads_by_umig_cbs.push_back(cb);
					reads_by_umig_umis.push_back(reads_per_umi.first);
					reads_by_umig.push_back(reads_per_umi.second);
				}
			}
		}
		return List::create(
				_["reads"] = wrap(reads_by_umig),
				_["cbs"] = wrap(reads_by_umig_cbs),
				_["umis"] = wrap(reads_by_umig_umis)
		);
	}

	DataFrame ResultsPrinter::get_reads_per_chr_per_cell_info(Stats::CellChrStatType stat_type,
	                                                          const CellsDataContainer &container) const
	{
		s_vec_t chr_cell_names, chr_names;
		i_vec_t reads_per_chr_per_cell;
		container.get_stat_by_real_cells(stat_type, chr_cell_names, chr_names, reads_per_chr_per_cell);

		return ResultsPrinter::create_matrix(chr_names, chr_cell_names, reads_per_chr_per_cell);
	}

	List ResultsPrinter::get_reads_per_chr_per_cell_info(const CellsDataContainer &container) const
	{
		L_TRACE << "Reads per chromosome per cell;";
		L_TRACE << "Fill exon results";
		auto exon = this->get_reads_per_chr_per_cell_info(Stats::EXON_READS_PER_CHR_PER_CELL, container);

		L_TRACE << "Fill intron results";
		auto intron = this->get_reads_per_chr_per_cell_info(Stats::INTRON_READS_PER_CHR_PER_CELL, container);

		L_TRACE << "Fill intergenic results";
		auto intergenic = this->get_reads_per_chr_per_cell_info(Stats::INTERGENIC_READS_PER_CHR_PER_CELL, container);

		return List::create(_["Exon"] = exon,
		                    _["Intron"] = intron,
		                    _["Intergenic"] = intergenic);
	}

	SEXP ResultsPrinter::get_count_matrix(const CellsDataContainer &container, bool filtered) const
	{
		if (filtered)
		{
			Tools::trace_time("Compiling filtered count matrix");
		}
		else
		{
			Tools::trace_time("Compiling raw count matrix");
		}

		Tools::init_r();

		s_vec_t gene_names, cell_names;
		SEXP cm;
		if (filtered)
		{
			cm = this->get_count_matrix_filtered(container);
		}
		else
		{
			cm = this->get_count_matrix_raw(container);
		}

		Tools::trace_time("Done");
		return cm;
	}

	void ResultsPrinter::trace_gene_counts(const CellsDataContainer &container) const
	{
		std::unordered_map<std::string, unsigned long> gene_counts_map;
		for (size_t cell_id : container.filtered_cells())
		{
			for (auto const &gm : container.cell(cell_id).genes())
			{
				gene_counts_map[container.gene_indexer().get_value(gm.first)] += gm.second.size();
			}
		}

		L_TRACE << "\n" << gene_counts_map.size() << " genes";

		using sip_t = std::pair<std::string, unsigned long>;
		std::vector<sip_t> gene_counts(gene_counts_map.begin(), gene_counts_map.end());
		std::sort(gene_counts.begin(), gene_counts.end(),
		          [](const sip_t &p1, const sip_t &p2) { return p1.second > p2.second; });

		std::ostringstream ss;
		ss << "top genes:\n";
		for (size_t i = 0; i < std::min(gene_counts.size(), ResultsPrinter::top_print_size); i++)
		{
			ss << gene_counts[i].first << '\t' << gene_counts[i].second << "\n";
		}

		L_TRACE << ss.str();
	}

	NumericVector ResultsPrinter::get_mean_reads_per_umi(const CellsDataContainer &container) const
	{
		L_TRACE << "Mean reads per UMI;";
		NumericVector reads_per_umis(container.real_cells_number());
		CharacterVector cell_names(container.real_cells_number());

		size_t out_id = 0;
		for (size_t cell_id = 0; cell_id < container.total_cells_number(); ++cell_id)
		{
			auto const &cell = container.cell(cell_id);
			if (!cell.is_real())
				continue;

			size_t umis_num = 0;
			double reads_per_umi = 0.0;
			for (auto const &gene_rec : cell.genes())
			{
				for (auto const &umi_rec : gene_rec.second.umis())
				{
					reads_per_umi += umi_rec.second.read_count;
				}

				umis_num += gene_rec.second.size();
			}
			reads_per_umi /= umis_num;
			reads_per_umis.at(out_id) = reads_per_umi;
			cell_names.at(out_id) = cell.barcode();
			out_id++;
		}

		reads_per_umis.attr("names") = cell_names;
		return reads_per_umis;
	}

	List ResultsPrinter::get_reads_per_umi_per_cell(const CellsDataContainer &container) const
	{
		L_TRACE << "Reads per UMI per gene;";
		using std::unordered_map;
		using std::string;
		unordered_map<string, unordered_map<string, unordered_map<string, unsigned>>> reads_per_umi_per_cell;
		for (auto cell_id : container.filtered_cells())
		{
			auto &cell_reads_p_umigs = reads_per_umi_per_cell[container.cell(cell_id).barcode()];
			for (auto const &gene_rpus : container.cell(cell_id).requested_reads_per_umi_per_gene())
			{
				auto &out_gene_umis = cell_reads_p_umigs[gene_rpus.first];
				for (auto const &umi_reads : gene_rpus.second)
				{
					out_gene_umis[umi_reads.first] = (unsigned) umi_reads.second;
				}
			}
		}

		return wrap(reads_per_umi_per_cell);
	}

	List ResultsPrinter::get_merge_targets(const CellsDataContainer &container) const
	{
		L_TRACE << "Merge targets;";
		std::unordered_map<std::string, std::string> merge_targets;
		for (size_t cell_from_id = 0; cell_from_id < container.total_cells_number(); ++cell_from_id)
		{
			auto const &barcode_from = container.cell(cell_from_id).barcode();
			auto const &barcode_to = container.cell(container.merge_targets()[cell_from_id]).barcode();
			merge_targets[barcode_from] = barcode_to;
		}

		return wrap(merge_targets);
	}

	SEXP ResultsPrinter::get_count_matrix_filtered(const CellsDataContainer &container) const
	{
		s_vec_t gene_names, cell_names;
		std::unordered_map<std::string, size_t> gene_ids;
		triplets_vec_t triplets;

		for (size_t column_num = 0; column_num < container.filtered_cells().size(); ++column_num)
		{
			auto const &cur_cell = container.cell(container.filtered_cells()[column_num]);
			cell_names.push_back(cur_cell.barcode());

			for (auto const &umis_per_gene : cur_cell.requested_umis_per_gene(this->reads_output))
			{
				auto gene_it = gene_ids.emplace(umis_per_gene.first, gene_ids.size());
				if (gene_it.second)
				{
					gene_names.push_back(umis_per_gene.first);
				}

				size_t row_num = gene_it.first->second;

				triplets.push_back(eigen_triplet_t(row_num, column_num, umis_per_gene.second));
			}
		}

		L_TRACE << gene_ids.size() << " genes, " << cell_names.size() << " cells.";
		return ResultsPrinter::create_matrix(triplets, gene_ids.size(), cell_names.size(), gene_names, cell_names);
	}

	SEXP ResultsPrinter::get_count_matrix_raw(const CellsDataContainer &container) const
	{
		s_vec_t gene_names, cell_names;
		std::unordered_map<std::string, size_t> gene_ids;
		triplets_vec_t triplets;

		size_t column_num = 0;
		for (size_t cell_id = 0; cell_id < container.total_cells_number(); cell_id++)
		{
			auto const &cell = container.cell(cell_id);
			if (!cell.is_real())
				continue;

			cell_names.push_back(cell.barcode());
			for (auto const &gene : cell.genes())
			{
				auto gene_it = gene_ids.emplace(container.gene_indexer().get_value(gene.first), gene_ids.size());
				if (gene_it.second)
				{
					gene_names.push_back(container.gene_indexer().get_value(gene.first));
				}

				size_t row_num = gene_it.first->second;
				size_t cell_value = gene.second.number_of_umis(this->reads_output);

				triplets.push_back(eigen_triplet_t(row_num, column_num, cell_value));
			}

			column_num++;
		}

		L_TRACE << gene_ids.size() << " genes, " << cell_names.size() << " cells.";
		return ResultsPrinter::create_matrix(triplets, gene_ids.size(), cell_names.size(), gene_names, cell_names);
	}

	IntegerVector ResultsPrinter::get_requested_umis_per_cb(const CellsDataContainer &container, bool return_reads) const
	{
		IntegerVector requested_umis_per_cb(container.real_cells_number());
		CharacterVector cell_names(container.real_cells_number());

		size_t real_cell_id = 0;
		for (size_t i = 0; i < container.total_cells_number(); ++i)
		{
			auto const &cell = container.cell(i);
			if (!cell.is_real())
				continue;

			cell_names[real_cell_id] = cell.barcode();

			size_t cell_expression = 0;
			if (return_reads)
			{
				for (auto const &gene : cell.requested_umis_per_gene(true))
				{
					cell_expression += gene.second;
				}
			}
			else
			{
				cell_expression = cell.requested_umis_num();
			}

			requested_umis_per_cb[real_cell_id] = cell_expression;
			real_cell_id++;
		}

		requested_umis_per_cb.attr("names") = cell_names;
		return requested_umis_per_cb;
	}

	SEXP ResultsPrinter::create_matrix(const triplets_vec_t &triplets, size_t total_rows, size_t total_cols,
	                                   const s_vec_t &row_names, const s_vec_t &col_names)
	{
		Eigen::SparseMatrix<unsigned> mat(total_rows, total_cols);
		mat.setFromTriplets(triplets.begin(), triplets.end());

		S4 res(wrap(mat));
		res.slot("Dimnames") = List::create(row_names, col_names);
		return res;
	}
}
