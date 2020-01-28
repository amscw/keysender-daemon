#ifndef _HW
#define _HW

#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include "common.h"
#include "mfhssioctl.h"
#include "exc.h"


struct hwExc_c : public exc_c
{
	enum class errCode_t : std::uint32_t {
		ERROR_OPEN,
		ERROR_IOCTL,
	} m_errCode;

	hwExc_c(enum errCode_t code, const std::string &strFile, const std::string &strFunction, const std::string &strErrorDescription = "") noexcept :
			exc_c(strFile, strFunction, strErrorDescription), m_errCode(code)
	{}

	const std::string &Msg() const noexcept override { return strErrorMessages[(int)m_errCode]; }
	void ToStderr() const noexcept override
	{
		std::cerr << "WTF:" << m_strFile << "(" << m_strFunction << "):" << strErrorMessages[(int)m_errCode] << "-" << m_strErrorDescription << std::endl;
	}

	std::string ToString() noexcept override
	{
		oss.str("");
		oss.clear();
		oss << "WTF:" << m_strFile << "(" << m_strFunction << "):" << strErrorMessages[(int)m_errCode] << "-" << m_strErrorDescription;
		return oss.str();
	}

	private:
		static std::string strErrorMessages[];
		std::ostringstream oss;
};


class hw_c
{
	int fd;
	std::string ifname;
	struct ifreq ifr;

public:
	hw_c(const std::string &ifn);
	~hw_c();

	bool IsOnline();
};

#endif // _HW
