#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "CxiBuilder.h"

#define die(msg) do { fputs(msg "\n\n", stderr); return 1; } while(0)
#define safe_call(a) do { int rc = a; if(rc != 0) return rc; } while(0)

#ifdef WIN32
static inline char* FixMinGWPath(char* buf)
{
	if (*buf == '/')
	{
		buf[0] = buf[1];
		buf[1] = ':';
	}
	return buf;
}
#else
#define FixMinGWPath(_arg) (_arg)
#endif

int usage(const char* prog)
{
	fprintf(stderr,
		"Usage: %s [options] input.3dsx output.cxi\n"
		"Options:\n"
		"  -n, --name=<value>      Specifies the process name of the application\n"
		"  -c, --code=<value>      Specifies the product code of the application\n"
		"  -t, --tid=<value>       Specifies the title ID of the application\n"
		"  -s, --settings=<file>   Specifies the settings file\n"
		"  -b, --banner=<file>     Specifies the banner file to embed in the CXI\n"
		"  -v, --version           Displays version information\n"
		"  -?, --help              Displays this text\n"
		, prog);
	return EXIT_FAILURE;
}

int main(int argc, char* argv[])
{
	const char* processName  = "homebrew";
	const char* productCode  = "CTR-HB-APP";
	const char* titleIdStr   = "000400000FF3FF00";
	const char* settingsFile = NULL;
	const char* bannerFile   = NULL;
	const char* inputFile    = NULL;
	const char* outputFile   = NULL;
	u64 titleId = 0;

	static struct option long_options[] =
	{
		{ "name",     required_argument, NULL, 'n' },
		{ "code",     required_argument, NULL, 'c' },
		{ "tid",      required_argument, NULL, 't' },
		{ "settings", required_argument, NULL, 's' },
		{ "banner",   required_argument, NULL, 'b' },
		{ "version",  no_argument,       NULL, 'v' },
		{ "help",     no_argument,       NULL, '?' },
		{ NULL, 0, NULL, 0 }
	};

	int opt, optidx = 0;
	while ((opt = getopt_long(argc, argv, "n:c:t:s:b:v?", long_options, &optidx)) != -1)
	{
		switch (opt)
		{
			case 'n': processName  = optarg; break;
			case 'c': productCode  = optarg; break;
			case 't': titleIdStr   = optarg; break;
			case 's': settingsFile = FixMinGWPath(optarg); break;
			case 'b': bannerFile   = FixMinGWPath(optarg); break;
			case 'v': printf("%s - Built on %s %s\n", PACKAGE_STRING, __DATE__, __TIME__); return EXIT_SUCCESS;
			case '?':        usage(argv[0]); return EXIT_SUCCESS;
			default:  return usage(argv[0]);
		}
	}

	if ((argc-optind) != 2)
		return usage(argv[0]);

	inputFile  = FixMinGWPath(argv[optind]);
	outputFile = FixMinGWPath(argv[optind+1]);

	if (strlen(processName) > 8)
	{
		fprintf(stderr, "Process name too long (max 8 characters): %s\n", processName);
		return EXIT_FAILURE;
	}

	if (strlen(productCode) > 16)
	{
		fprintf(stderr, "Product code too long (max 16 characters): %s\n", productCode);
		return EXIT_FAILURE;
	}

	titleId = strtoull(titleIdStr, NULL, 16);
	if ((titleId>>48) != 4)
	{
		fprintf(stderr, "Not a valid 3DS title ID: %08X%08X\n", (u32)(titleId>>32), (u32)titleId);
		return EXIT_FAILURE;
	}

	CxiBuilder builder(processName, productCode, titleId, 0);
	if (settingsFile)
		safe_call(builder.ReadSettings(settingsFile));
	safe_call(builder.Read3DSX(inputFile));
	if (bannerFile)
		safe_call(builder.ReadBanner(bannerFile));
	safe_call(builder.FinishConfig());
	safe_call(builder.BuildExeFS());
	safe_call(builder.Write(outputFile));

	return EXIT_SUCCESS;
}
