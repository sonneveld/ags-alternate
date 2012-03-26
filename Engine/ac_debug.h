#ifndef _AC_DEBUG_H_HEADER
#define _AC_DEBUG_H_HEADER

extern bool send_message_to_editor(const char *msg);
extern bool send_message_to_editor(const char *msg, const char *errorMsg);
extern const char *get_cur_script(int numberOfLinesOfCallStack);
extern void check_debug_keys() ;
extern bool init_editor_debugging();
extern void install_debug_handlers();

#endif
