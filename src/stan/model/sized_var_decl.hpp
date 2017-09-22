#ifndef STAN_MODEL_SIZED_VAR_DECL_HPP
#define STAN_MODEL_SIZED_VAR_DECL_HPP

#include <stan/model/var_decl.hpp>
#include <string>
#include <vector>

namespace stan {
  namespace model {

    /**
     * Runtime variable declaration information including the name of
     * the variable, the name of its type, number of array dimensions,
     * and the sizes of all of the dimensions (including vector and
     * matrix dimensions, with array dimensions first).
     */
    class sized_var_decl : public var_decl {
    private:
      const std::vector<int> sizes_;

    public:
      /**
       * Construct a variable declaration with the specified name and
       * type name, array dimensions, lower and upper bound indicator
       * flags, dimension sizes, and lower and upper bounds.
       *
       * @param[in] name name of variable
       * @param[in] type_name name of the variable's type
       * @param[in] array_dims number of array dimensions
       * @param[in] has_lb true if declaration has lower
       * bound
       * @param[in] has_ub true if declaration has upper
       * bound
       * @param[in] sizes sizes of dimensions, including array and
       * matrix dimensions
       */
      sized_var_decl(const std::string& name, const std::string& type_name,
                     int array_dims, bool has_lb, bool has_ub,
                     const std::vector<int>& sizes)
        : var_decl(name, type_name, array_dims, has_lb, has_ub),
          sizes_(sizes) { }

      /**
       * Return the sizes for the intrinsic and array dimensions for
       * this variable declaration.  The dimensions are listed in the
       * order of indexing, first the array dimensions then any vector
       * or matrix dimensions.
       *
       * The lifespan of the reference is the same as for this
       * class.
       *
       * @return name of variable declared
       */
      const std::vector<int>& sizes() const {
        return sizes_;
      }
    };

  }
}
#endif