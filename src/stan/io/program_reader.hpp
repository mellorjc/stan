#ifndef STAN_IO_PROGRAM_READER_PROGRAM_READER_HPP
#define STAN_IO_PROGRAM_READER_PROGRAM_READER_HPP

#include <stan/io/read_line.hpp>
#include <stan/io/starts_with.hpp>
#include <cstdio>
#include <istream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <iostream>  // TODO(carpenter): remove this line

namespace stan {
  namespace io {

    /**
     * Structure to hold preprocessing events, which hold (a) line
     * number in concatenated program after includes, (b) line number
     * in the stream from which the text is read, (c) a string-based
     * action, and (d) a path to the current file.
     */
    struct preproc_event {
      int concat_line_num_;
      int line_num_;
      std::string action_;
      std::string path_;
      preproc_event(int concat_line_num, int line_num,
                    const std::string& action, const std::string& path)
        : concat_line_num_(concat_line_num), line_num_(line_num),
          action_(action), path_(path) { }
    };

    /**
     * A <code>program_reader</code> reads a Stan program and unpacks
     * the include statements relative to a search path in such a way
     * that error messages can reproduce the include path.
     */
    class program_reader {
    public:

      /**
       * A path/position pair.
       */
      typedef std::pair<std::string, int> dump_t;

      /**
       * Sequence of path/position pairs.
       */
      typedef std::vector<dump_t> dumps_t;

      /**
       * Construct a program reader from the specified stream derived
       * from the specified name or path, and a sequence of paths to
       * search for include files.  The paths should be directories.
       *
       * <p>It is up to the caller that created the input stream to
       * close it.
       *
       * @param[in] in stream from which to read
       * @param[in] name name or path attached to stream
       * @param[in] search_path ordered sequence of directory names to
       * search for included files
       */
      program_reader(std::istream& in, const std::string& name,
                     const std::vector<std::string>& search_path) {
        int concat_line_num = 0;
        read(in, name, search_path, concat_line_num);
      }

      /**
       * Return a stream from which to read the concatenated program.
       * Modifying the stream will modify the underlying class.
       *
       * @return stream for program
       */
      std::stringstream& program_stream() {
        return program_;
      }

      /**
       * Return the include message for the target line number.  This
       * will take the form
       *
       * <pre>
       * in file '<file>' at line <num>
       * included from file '<file>' at line <num>
       * ...
       * included from file '<file> at line <num>
       * </pre>
       *
       * @param target_line_num line number in concatenated program
       * @return include trace for the line number
       * @throw std::runtime_exception if the include stack is empty
       * or the target line number is less than 1
       */
      std::string include_trace(int target_line_num) const {
        const dumps_t x = include_stack(target_line_num);
        if (x.size() < 1 || target_line_num < 1) {
          std::stringstream ss;
          ss << "Target line number " << target_line_num << " not found."
             << std::endl;
          std::string error_msg = ss.str();
          throw std::runtime_error(error_msg);
        }
        std::stringstream ss;
        ss << "in file '" << x[x.size() - 1].first
           << "' at line " << x[x.size() - 1].second
           << std::endl;
        for (size_t i = x.size() - 1; i-- > 0; ) {
          ss << "included from file '" << x[i].first
             << "' at line " << x[i].second
             << std::endl;
        }
        return ss.str();
      }

      /**
       * Return the include trace of the path and line numbers leading
       * to the specified line of text in the concatenated program.
       *
       * @param[in] target line number in concatenated program file
       * @return sequence of files and positions for includes
       */
      dumps_t include_stack(int target) const {
        dumps_t result;
        std::string file = "ERROR: UNINITIALIZED";
        int file_start = -1;
        int concat_start = -1;
        for (size_t i = 0; i < history_.size(); ++i) {
          if (target <= history_[i].concat_line_num_) {
            int line = file_start + target - concat_start;
            result.push_back(dump_t(file, line));
            return result;
          } else if (history_[i].action_ == "start"
                     || history_[i].action_ == "restart" ) {
            file = history_[i].path_;
            file_start = history_[i].line_num_;
            concat_start = history_[i].concat_line_num_;
          } else if (history_[i].action_ == "end") {
            if (result.size() == 0) break;
            result.pop_back();
          } else if (history_[i].action_ == "include") {
            result.push_back(dump_t(file, history_[i].line_num_ + 1));
          }
        }
        return dumps_t();
      }

      // TODO(carpenter): remove this debug function before releasing
      void print_history(std::ostream& out) {
        for (size_t i = 0; i < history_.size(); ++i)
          out << i << ". (" << history_[i].concat_line_num_
              << ", " << history_[i].line_num_
              << ", " << history_[i].action_
              << ", " << history_[i].path_ << ")"
              << std::endl;
      }


    private:
      std::stringstream program_;
      std::vector<preproc_event> history_;

      /**
       * Returns the characters following <code>#include</code> on
       * the line, trimming whitespace characters.  Assumes that
       * <code>#include</code>" is line initial.
       *
       * @param line line of text beginning with <code>#include</code>
       * @return text after <code>#include</code> with whitespace
       * trimmed
       */
      static std::string include_path(const std::string& line) {
        int start = std::string("#include").size();
        while (line[start] == ' ') ++start;
        int end = line.size() - 1;
        while (line[end] == ' ') --end;
        return line.substr(start, end - start);
      }

      /**
       * Read the rest of a program from the specified input stream in
       * the specified path, with the specified search path for
       * include files, and incrementing the specified concatenated
       * line number.  This method is called recursively for included
       * files.
       *
       * @param[in] in stream from which to read
       * @param[in] path name of stream
       * @param[in] search_path sequence of path names to search for
       * include files
       * @param[in,out] concat_line_num position in concatenated file
       * to be updated
       * @throw std::runtime_error if an included file cannot be found
       */
      void read(std::istream& in, const std::string& path,
                const std::vector<std::string>& search_path,
                int& concat_line_num) {
        history_.push_back(preproc_event(concat_line_num, 0, "start", path));
        for (int line_num = 1; ; ++line_num) {
          std::string line = read_line(in);
          if (line.empty()) {
            // ends initial out of loop start event
            history_.push_back(preproc_event(concat_line_num, line_num - 1,
                                             "end", path));
            break;
          } else if (starts_with("#include ", line)) {
            std::string incl_path = include_path(line);
            history_.push_back(preproc_event(concat_line_num, line_num - 1,
                                             "include", incl_path));
            bool found_path = false;
            for (size_t i = 0; i < search_path.size(); ++i) {
              std::string f = search_path[i] + incl_path;
              std::ifstream include_in(f.c_str());
              if (!include_in.good()) {
                include_in.close();
                continue;
              }
              try {
                read(include_in, incl_path, search_path, concat_line_num);
              } catch (...) {
                include_in.close();
                throw;
              }
              include_in.close();
              history_.push_back(preproc_event(concat_line_num, line_num,
                                               "restart", path));
              found_path = true;
              break;
            }
            if (!found_path)
              throw std::runtime_error("could not find include file");
          } else {
            ++concat_line_num;
            program_ << line;
          }
        }
      }

    };

  }
}
#endif

