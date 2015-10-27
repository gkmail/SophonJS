var num = Number(arguments[1]);

function fib(n)
{
	if (n > 2)
		return fib(n - 2) + fib(n - 1);
	else
		return n;
}

if (!isNaN(num))
	console.log("result: %d", fib(num));
