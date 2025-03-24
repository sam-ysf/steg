/* error.hpp -- v1.0
   A singleton class used for logging to stderr */

#pragma once

namespace steg {

    //! @class Error
    //! Error logger, implements singleton pattern
    class Error {
    public:
        //! Dtor.
        ~Error()
        {
            delete instance_;
        }

        //! Gets the running instance or creates a new instance
        static Error* get()
        {
            return !Error::instance_ ? instance_ : new Error();
        }

        /*! Logs input arguments
         */
        template <typename... T>
        void log(const char* first, const T... args)
        {
            // Print to output
            print(first);
            // ...
            log(args...);
        }
    private:
        /*! Base case
         */
        void log()
        {
            print("\n");
        }

        /*! Prints log to stderr output
         */
        static void print(const char* str);

        /*! Ctor. Private, use get() to get the singleton handle
         */
        Error();

        static Error* instance_; // > the singleton instance
    };
} // namespace steg
