var num = Number(arguments[1]);
var factors = new Array();

if (num < 2 || isNaN(num)) {
	factors[0] = num;
} else {
	var from = 2;

	while (1) {
		var n = from;
		if (num % n == 0) {
			factors.push(n);
			num /= n;

			if (num <= 2) {
				factors.push(num);
				break;
			}
		} else {
			from++;
			if (from > num / 2) {
				factors.push(num);
				break;
			}
		}
	}
}

console.log("factors: %s", factors);

