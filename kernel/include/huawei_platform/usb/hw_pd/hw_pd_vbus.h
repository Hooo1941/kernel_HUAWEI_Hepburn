
#ifndef HW_PD_VBUS_H
#define HW_PD_VBUS_H

/* used to indicate which event sent a completion message */
enum pd_wait_typec_complete {
	NOT_COMPLETE,
	COMPLETE_FROM_VBUS_DISCONNECT,
	COMPLETE_FROM_TYPEC_CHANGE,
};

void reinit_typec_completion(void);
void typec_complete(enum pd_wait_typec_complete typec_completion);
void pogopin_set_pmic_vbus_irq_enable(int enable);
int pmic_vbus_irq_is_enabled(void);
bool pmic_vbus_is_connected(void);
void pmic_vbus_disconnect_process(void);

#endif

