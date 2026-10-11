/* error.hpp -- v1.0
   A singleton class used for logging to stderr */

#pragma once

#include <memory>
#include <mutex>

namespace steg {

    //! @class Error
    //! Error logger, implements singleton pattern
    class Error {
    public:
        //! Gets the running singleton instance
        //! Creates a new instance if none exists
        static Error* get()
        {
            std::scoped_lock lock(acces_lock_);

            if (!instance_) {
                instance_ = std::make_shared<Error>();
            }

            return instance_.get();
        }

        template <typename... Types>
        void log(const char* first, const Types... args) const
        {
            // Print to output
            fprintf(stderr, "%s ", first);
            // Rest of args...
            log(args...);
        }
    private:
        /* Base case
         */
        static void log()
        {
            fprintf(stderr, "\n");
        }

        /* Prints log to stderr output
         */
        static void print(const char* str);

        /* Just in case
         */
        static std::mutex acces_lock_;

        /* Singleton instnace
         */
        static std::shared_ptr<Error> instance_;
    };
} // namespace steg
