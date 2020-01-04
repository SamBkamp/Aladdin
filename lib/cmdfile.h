int init();
int test_command(const char *test_cmd, char *outputMsg, int maxOutputLen);
int finish();
int remove_command(char* remove_cmd);
void list_bot_commands();
int add_command(char* add_cmd, char* add_msg);
