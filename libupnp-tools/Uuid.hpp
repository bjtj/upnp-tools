#ifndef __UUID_HPP__
#define __UUID_HPP__

#include <string>

namespace UPNP {
    
	/**
	 * @brief uuid
	 * @ref https://tools.ietf.org/html/rfc4122 (there's sample source)
	 * TODO: implement spec.
	 */
    class Uuid {
    private:
        std::string uuid;
        std::string rest;
        
    public:
        Uuid(const std::string & uuid);
        virtual ~Uuid();
        void clear();
        void parse(const std::string & uuid);
        std::string getUuid() const;
        std::string getRest() const;
        void setUuid(const std::string & uuid);
        void setRest(const std::string & rest);
        static void testValidFormat(const std::string & uuid);
        std::string toString() const;
    };

	/**
	 * @brief
	 */
	class UuidGenerator {
	private:
	public:
		UuidGenerator();
		virtual ~UuidGenerator();
		virtual std::string generate() = 0;
	};

	/**
	 * @brief
	 */
	class UuidGeneratorDefault : public UuidGenerator {
	private:
		unsigned long nonce;
	public:
		UuidGeneratorDefault();
		virtual ~UuidGeneratorDefault();
		virtual std::string generate();
	};

}

#endif
