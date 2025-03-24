/* error.hpp -- v1.0
   A singleton class used for logging to stderr */

#pragma once

#include <memory>

namespace steg {

    //! @class Error
    //! Error logger, implements singleton pattern
    class Error {
    public:
        //! Gets the running singleton instance
        //! Creates a new instance if none exists
        static Error* get()
        {
            if (!instance_)
                instance_ = std::make_shared<Error>();
            return instance_.get();
        }

        /*! Logs input arguments
         */
        template <typename... T>
        void log(const char* first, const T... args) const
        {
            // Print to output
            print(first);
            // ...
            log(args...);
        }
    private:
        /*! Base case
         */
        void log() const
        {
            print("\n");
        }

        /*! Prints log to stderr output
         */
        static void print(const char* str);

        static std::shared_ptr<Error> instance_; // > the singleton instance
    };
} // namespace steg
