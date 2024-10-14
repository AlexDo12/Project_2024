#include <vector>

using std::vector;

// ============ Defines ============
#define MASTER 0
#define FROM_MASTER 1          /* setting a message type */
#define FROM_WORKER 2          /* setting a message type */


bool is_vec_sorted(vector<int> values, int taskid);
void check_sorted(vector<int> data, const char* sort_type, const char* name, int taskid);
vector<int> generate_vector(int length, int num_processes, int taskid, int order);
