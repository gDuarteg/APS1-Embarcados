#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"
#include "note.h"

#define LED_PIO					PIOC					
#define LED_PIO_ID				ID_PIOC					
#define LED_PIO_IDX				8						
#define LED_PIO_IDX_MASK		(1u << LED_PIO_IDX)		

#define BUT1_PIO				PIOD
#define BUT1_PIO_ID				ID_PIOD
#define BUT1_PIO_IDX			28
#define BUT1_PIO_IDX_MASK		(1u << BUT1_PIO_IDX)
#define BUT1_PRIORITY			6

#define BUT2_PIO				PIOC
#define BUT2_PIO_ID				12
#define BUT2_PIO_IDX			31
#define BUT2_PIO_IDX_MASK		(1u << BUT2_PIO_IDX)
#define BUT2_PRIORITY			4

#define BUT3_PIO				PIOA
#define BUT3_PIO_ID				ID_PIOA
#define BUT3_PIO_IDX			19
#define BUT3_PIO_IDX_MASK		(1u << BUT3_PIO_IDX)
#define BUT3_PRIORITY			4

#define BUZ_PIO					PIOC						
#define BUZ_PIO_ID				ID_PIOC						
#define BUZ_PIO_IDX				13							
#define BUZ_PIO_IDX_MASK		(1u << BUZ_PIO_IDX)			

typedef struct
{
	int *notes;
	int *times;
	size_t len;
	char *title;
} song;

int but1 = 0;
int but2 = 0;
int but3 = 0;

void BUT1_callback(void)
{
	but1 = 1;
}

void BUT2_callback(void)
{
	but2 = 1;
}

void BUT3_callback(void)
{
	but3 = 1;
}

void init(void)
{
	sysclk_init();
	WDT->WDT_MR = WDT_MR_WDDIS;
	pmc_enable_periph_clk(LED_PIO_ID);
	pio_set_output(LED_PIO, LED_PIO_IDX_MASK, 0, 0, 0);
	pmc_enable_periph_clk(BUT1_PIO_ID);
	pmc_enable_periph_clk(BUT2_PIO_ID);
	pmc_enable_periph_clk(BUT3_PIO_ID);
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK, PIO_PULLUP);
	pio_configure(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK, PIO_PULLUP);
	pio_configure(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK, PIO_PULLUP);
	NVIC_EnableIRQ(BUT1_PIO_ID);
	NVIC_SetPriority(BUT1_PIO_ID, BUT1_PRIORITY);
	NVIC_EnableIRQ(BUT2_PIO_ID);
	NVIC_SetPriority(BUT2_PIO_ID, BUT2_PRIORITY);
	NVIC_EnableIRQ(BUT3_PIO_ID);
	NVIC_SetPriority(BUT3_PIO_ID, BUT3_PRIORITY);
	pio_enable_interrupt(BUT1_PIO, BUT1_PIO_IDX_MASK);
	pio_enable_interrupt(BUT2_PIO, BUT2_PIO_IDX_MASK);
	pio_enable_interrupt(BUT3_PIO, BUT3_PIO_IDX_MASK);
	pmc_enable_periph_clk(BUZ_PIO_ID);
	pio_set_output(BUZ_PIO, BUZ_PIO_IDX_MASK, 0, 0, 0);
	pio_handler_set(BUT1_PIO, BUT1_PIO_ID, BUT1_PIO_IDX_MASK, PIO_IT_FALL_EDGE, BUT1_callback);
	pio_handler_set(BUT2_PIO, BUT2_PIO_ID, BUT2_PIO_IDX_MASK, PIO_IT_FALL_EDGE, BUT2_callback);
	pio_handler_set(BUT3_PIO, BUT3_PIO_ID, BUT3_PIO_IDX_MASK, PIO_IT_FALL_EDGE, BUT3_callback);
}

int main (void)
{
	board_init();
	sysclk_init();
	init();
	delay_init();

	int current = 0;
	int pause = 1;
	size_t step = 0;
	
	song song1, song2, current_song;
	
	song1.notes = n1;
	song1.times = t1;
	song1.len = sizeof(n1) / sizeof(n1[0]);
	song1.title = &"MARIO 1";
	
	song2.notes = n2;                         
	song2.times = t2;                        
	song2.len = sizeof(n2) / sizeof(n2[0]);
	song2.title = &"MARIO 2";

	song songs[] = {song1, song2};
	current_song = songs[0];
	
	gfx_mono_ssd1306_init();
	gfx_mono_draw_string(songs[current].title, 10, 10, &sysfont);

	while (1)
	{
		if (pause == 1) {
			pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
		}
		
		if (but1){
			but1 = 0;
			pause = 0;
		}
		if (but2){
			but2 = 0;
			pause = 1;
		}

		if (but3){
			but3 = 0;
			if (current == 0)
			{
				current = 1;
				current_song = songs[current];
			} else {
				current = 0;
				current_song = songs[current];
			}
			
			step = 0;
			gfx_mono_draw_string("		", 10, 10, &sysfont);
			gfx_mono_draw_string(songs[current].title, 10, 10, &sysfont);
		}

		if (!pause){
			if (step < current_song.len){
				int note_time = 800 / current_song.times[step];				
				int f = current_song.notes[step];
				int delay_time = 500000 / f;
				
				int n_f = (note_time * f) / 1000;
				for (int i = 0; i < n_f; i++)
				{
					pio_set(PIOC, LED_PIO_IDX_MASK);   
					pio_set(PIOC, BUZ_PIO_IDX_MASK);   
					delay_us(delay_time);					   
					pio_clear(PIOC, LED_PIO_IDX_MASK); 
					pio_clear(PIOC, BUZ_PIO_IDX_MASK); 
					delay_us(delay_time);
				}
				delay_ms(note_time);
				
				step++;
			} else {
				pause = 0;
				step = 0;
			}
		}
	}
	return 0;
}