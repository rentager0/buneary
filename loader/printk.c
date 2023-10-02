#include <loader/loader.h>
#include <loader/printk.h>
#include <loader/string.h>
#include <loader/vga.h>


size_t printk(const char *format, ...)
{
	va_list ap;
	size_t ret;

	va_start(ap, format);
	ret = vprintk(format, ap);
	va_end(ap);

	return ret;
}

size_t snprintk(char *buffer, size_t len, const char *format, ...)
{
	va_list ap;
	size_t ret;

	va_start(ap, format);
	ret = vsnprintk(buffer, len, format, ap);
	va_end(ap);

	return ret;
}

size_t hprintk(bool_t handler(void *, char), void *u, const char *format, ...)
{
	va_list ap;
	size_t ret;

	va_start(ap, format);
	ret = vhprintk(handler, u, format, ap);
	va_end(ap);

	return ret;
}


static bool_t vprintk_handler(void *screen, char c)
{
	struct vga *__screen = (struct vga *) screen;

	vga_putc(__screen, c);
	return TRUE;
}

size_t vprintk(const char *format, va_list ap)
{
	return vhprintk(vprintk_handler, &this.screen, format, ap);
}


struct vsnprintk_state
{
	char *buffer;
	size_t len;
	size_t ptr;
};

static bool_t vsnprintk_handler(void *user, char c)
{
	struct vsnprintk_state *state = (struct vsnprintk_state *) user;
	
	if (state->ptr >= state->len)
		return 0;
	
	state->buffer[state->ptr] = c;
	state->ptr++;

	return 1;
}

size_t vsnprintk(char *buffer, size_t len, const char *format, va_list ap)
{
	struct vsnprintk_state state;

	state.buffer = buffer;
	state.len = len;
	state.ptr = 0;

	return vhprintk(vsnprintk_handler, (void *) &state, format, ap);
}


struct vhprintk_format
{
	bool_t  alternate_form;
	bool_t  zero_pad;
	bool_t  right_pad;
	bool_t  positive_blank;
	bool_t  visible_sign;
	bool_t  long_operand;
	size_t  minimum_size;
	char    type;
};

struct vhprintk_state
{
	const char               *input;
	bool_t                  (*handler)(void *, char);
	void                     *user;
	struct vhprintk_format    format;
	size_t                    done;
	bool_t                    stop;
};


static size_t number_length(size_t number, uint8_t base)
{
	size_t len = 0;

	do {
		number /= base;
		len++;
	} while (number > 0);

	return len;
}

static bool_t vhprintk_print_one(struct vhprintk_state *state, char c)
{
	if (!state->stop) {
		if (!state->handler(state->user, c))
			state->stop = FALSE;
		else
			state->done++;
	}

	return !(state->stop);
}

static void vhprintk_pad(struct vhprintk_state *state, size_t len, char pad)
{
	size_t i;

	for (i = 0; i < len; i++)
		if (!vhprintk_print_one(state, pad))
			return;
}

static void vhprintk_print_radical(struct vhprintk_state *state,
				   uint32_t radical, uint8_t base, size_t len,
				   char upchar)
{
	size_t div = 1;
	size_t unit;
	char c;

	while (len > 1) {
		div *= base;
		len--;
	}

	do {
		unit = radical / div;
		if (unit < 10)
			c = '0' + unit;
		else
			c = upchar + (unit - 10);

		if (!vhprintk_print_one(state, c))
			return;

		radical = radical % div;
		div /= base;
	} while (div > 0);
}

static void vhprintk_read_format(struct vhprintk_state *state)
{
	struct vhprintk_format *format = &state->format;
	const char *input = state->input;
	char c;

	memset(format, 0, sizeof (struct vhprintk_format));

 	while ((c = *(input++)) != '\0') {
 		switch (c) {
 		case '#':
 			format->alternate_form = TRUE;
 			break;
 		case '0':
 			format->zero_pad = TRUE;
 			break;
 		case '-':
 			format->right_pad = TRUE;
 			break;
 		case ' ':
 			format->positive_blank = TRUE;
 			break;
 		case '+':
 			format->visible_sign = TRUE;
 			break;
 		default:
 			input--;
 			goto next;
 		}
 	}

 	goto out;

 next:
	while ((c = *(input++)) != '\0') {
		if (c >= '0' && c <= '9') {
			format->minimum_size =
				format->minimum_size * 10 + (c - '0');
		} else {
			format->type = c;
			break;
		}
	}

 out:
	if (format->type == 'l') {
		format->long_operand = TRUE;
		format->type = *(input++);
	}

	state->input = input;
}


static void vhprintk_print_number(struct vhprintk_state *state,
				  uint64_t positive, uint8_t base,
				  const char *prefix, char upbase)
{
	size_t len = number_length(positive, base);
	size_t preflen = strlen(prefix);
	size_t totlen = len + preflen;
	size_t padlen = 0;
	size_t i;

	if ((positive >> 32) != 0) {
		printk("<too big>");
		return;
	}

	if (state->format.minimum_size > totlen)
		padlen = state->format.minimum_size - totlen;

	if (!state->format.right_pad && !state->format.zero_pad)
		vhprintk_pad(state, padlen, ' ');

	for (i = 0; i < preflen; i++)
		vhprintk_print_one(state, prefix[i]);
	
	if (!state->format.right_pad && state->format.zero_pad)
		vhprintk_pad(state, padlen, '0');

	vhprintk_print_radical(state, positive, base, len, upbase);

	if (state->format.right_pad)
		vhprintk_pad(state, padlen, ' ');
}

static void vhprintk_print_signed(struct vhprintk_state *state, uint8_t base,
				  int64_t arg)
{
	const char *prefix = "";
	uint64_t positive;

	if (arg < 0) {
		positive = -arg;
		prefix = "-";
	} else {
		positive = arg;
		if (state->format.positive_blank)
			prefix = " ";
		if (state->format.visible_sign)
			prefix = "+";
	}

	vhprintk_print_number(state, positive, base, prefix, 'a');
}

static void vhprintk_print_unsigned(struct vhprintk_state *state, uint8_t base,
				    char upbase, uint64_t arg)
{
	const char *prefix = "";

	if (state->format.type == 'x' || state->format.type == 'X') {
		if (state->format.alternate_form)
			prefix = "0x";
	} else if (state->format.type == 'o') {
		if (state->format.alternate_form)
			prefix = "0";
	} else if (state->format.type == 'b') {
		if (state->format.alternate_form)
			prefix = "0b";
	} else {
		if (state->format.positive_blank)
			prefix = " ";
		if (state->format.visible_sign)
			prefix = "+";
	}

	vhprintk_print_number(state, arg, base, prefix, upbase);
}

static void vhprintk_print_char(struct vhprintk_state *state, uint32_t arg)
{
	size_t padlen = 0;
	size_t len = 1;

	if (state->format.minimum_size > len)
		padlen = state->format.minimum_size - len;

	if (!state->format.right_pad)
		vhprintk_pad(state, padlen, ' ');

	vhprintk_print_one(state, arg);
	
	if (state->format.right_pad)
		vhprintk_pad(state, padlen, ' ');
}

static void vhprintk_print_string(struct vhprintk_state *state,
				  const char *arg)
{
	size_t padlen = 0;
	size_t len, i;

	if (arg == NULL)
		arg = "(null)";

	len = strlen(arg);

	if (len == 0 && state->format.positive_blank)
		padlen = 1;
	if (state->format.minimum_size > len)
		padlen = state->format.minimum_size - len;

	if (!state->format.right_pad)
		vhprintk_pad(state, padlen, ' ');

	for (i = 0; i < len; i++)
		vhprintk_print_one(state, *arg++);
	
	if (state->format.right_pad)
		vhprintk_pad(state, padlen, ' ');
}

size_t vhprintk(bool_t handler(void *, char), void *user, const char *input,
		va_list ap)
{
	struct vhprintk_state state;
	bool_t fmt = FALSE;
	uint8_t base;
	char upbase;
	char c;

	memset(&state, 0, sizeof (struct vhprintk_state));
	state.input = input;
	state.handler = handler;
	state.user = user;

	while ((c = *(input++)) != '\0') {
		if (state.stop)
			break;
		
		if (c == '%') {
			if (fmt) {
				fmt = FALSE;
			} else {
				fmt = TRUE;
				continue;
			}
		}

		if (!fmt) {
			vhprintk_print_one(&state, c);
			continue;
		}

		state.input = input - 1;
		vhprintk_read_format(&state);

		base = 0;
		upbase = 'a';

		if (state.format.type == 'p') {
			state.format.type = 'x';
			state.format.alternate_form = TRUE;
		}

		switch (state.format.type) {
		case 'c':
			vhprintk_print_char(&state, va_arg(ap, uint32_t));
			break;
		case 'd':
		case 'i':
			if (state.format.long_operand)
				vhprintk_print_signed(&state, 10,
						      va_arg(ap, int64_t));
			else
				vhprintk_print_signed(&state, 10,
						      va_arg(ap, int32_t));
			break;
		case 's':
			vhprintk_print_string(&state, va_arg(ap, char *));
			break;
		case 'X':
			upbase = 'A';
		case 'x':
			base += 6;
		case 'u':
			base += 2;
		case 'o':
			base += 6;
		case 'b':
			base += 2;
			if (state.format.long_operand)
				vhprintk_print_unsigned(&state, base, upbase,
							va_arg(ap, uint64_t));
			else
				vhprintk_print_unsigned(&state, base, upbase,
							va_arg(ap, uint32_t));
			break;
		}

		fmt = FALSE;
		input = state.input;
	}

	return state.done;
}
