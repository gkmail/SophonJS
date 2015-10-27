function fail()
{
	throw Error("test failed");
}

function constructor_test()
{
	function test(v, n)
	{
		if (v !== n)
			fail();
	}

	test(Number(),0);
	test(Number(1.234), 1.234);
	test(Number("0xff"), 255);
	test(Number("-3.1415926"), -3.1415926);

	var r;

	r = new Number();
	if (r === 0)
		fail();
	if (r != 0)
		fail();

	r = new Number(1.234);
	if (r === 1.234)
		fail();
	if (r != 1.234)
		fail();

	r = new Number("0xff");
	if (r === 255)
		fail();
	if (r != 255)
		fail();

	console.log("constructor test passed");
}

constructor_test();

function property_test()
{
	if (!isNaN(Number.NaN))
		fail();

	if (isFinite(Number.NEGATIVE_INFINITY))
		fail();
	if (Number.NEGATIVE_INFINITY >= 0)
		fail();

	if (isFinite(Number.POSITIVE_INFINITY))
		fail();
	if (Number.POSITIVE_INFINITY <= 0)
		fail();

	console.log("property test passed");
}

property_test();

function prototype_test()
{
	if ((255).toString()!="255")
		fail();
	if ((-255).toString()!="-255")
		fail();
	if ((1234567).toString(2)!="100101101011010000111")
		fail();
	if ((1234567).toString(5)!="304001232")
		fail();
	if ((1234567).toString(8)!="4553207")
		fail();
	if ((1234567).toString(10)!="1234567")
		fail();
	if ((1234567).toString(16)!="12d687")
		fail();
	if ((1234567).toString(36)!="qglj")
		fail();

	if ((123456.789).toFixed() != "123456")
		fail();
	if ((123456.789).toFixed(3) != "123456.789")
		fail();
	if ((123456.789).toFixed(5) != "123456.78900")
		fail();

	console.log("%s", (NaN).toFixed());
	console.log("%s", (NaN).toExponential());
	console.log("%s", (NaN).toPrecision());

	console.log("%s", (Infinity).toFixed());
	console.log("%s", (Infinity).toExponential());
	console.log("%s", (Infinity).toPrecision());

	console.log("%s", (-Infinity).toFixed());
	console.log("%s", (-Infinity).toExponential());
	console.log("%s", (-Infinity).toPrecision());

	console.log("%s", (12345.6789).toFixed());
	console.log("%s", (12345.6789).toExponential());
	console.log("%s", (12345.6789).toPrecision());

	console.log("%s", (1.2345).toString());

	console.log("prototype test passed");
}

prototype_test();

console.log("all tests passed");
