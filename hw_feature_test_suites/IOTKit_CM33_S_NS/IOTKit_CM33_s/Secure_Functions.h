/*----------------------------------------------------------------------------
 * Name:    Secure_Functions.h
 * Purpose: Function and Typedef Declarations to include into NonSecure Application.
 *----------------------------------------------------------------------------*/

/* Define typedef for NonSecure callback function */
typedef int32_t (*NonSecure_funcptr)(uint32_t);

/* Function declarations for Secure functions called from NonSecure application */
extern int32_t Secure_test_add_ns_callback(NonSecure_funcptr);
extern void Secure_lcd_print(uint32_t, char *);

extern int32_t test_cases_from_ns(uint32_t);
extern void test_sg_ns_s();
extern uint32_t test_add_s(uint32_t, uint32_t);
extern void test_state_switch_s();

extern int mtb_resume(size_t mtb_size, int pos);
extern int mtb_pause();

// test instructions after sg in NSC region
extern void test_nsc_mod();
extern void test_dwt_ns_s();