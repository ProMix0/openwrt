#define DRIVERVERSION	"v2.2.1.9 2024/11/12"

/*major-minor-hotfix-[branchid + hotfix]*/
#define CORE_VERSION(a, b, c, d) (((a) << 48) + ((b) << 32) + ((c) << 16) + (d))

#define CORE_MAJOR_VER	0002
#define CORE_MINOR_VER	0002
#define CORE_HOTFIX_VER	0001
#define CORE_BRANCH_IDX	0009

#define CORE_VER_CODE	CORE_VERSION(CORE_MAJOR_VER, CORE_MINOR_VER, CORE_HOTFIX_VER, CORE_BRANCH_IDX)

