function fail()
{
	throw Error("test failed");
}

function stringify_test()
{
	function test(o, str)
	{
		var r = JSON.stringify(o);

		console.log("%s", r);

		if (str !== null)
			if (r != str)
				fail();
	}

	test(true,"true");
	test(false,"false");
	test(null,"null");
	test(undefined,"null");
	test(12345,"12345");
	test("sophon js","\"sophon js\"");
	test([1,2,3,4,5],"[1,2,3,4,5]");
	test({a:1,b:false,c:"test"},null)
	test({a:1,b:false,c:{a:123,b:321},d:[{a:666,b:777},[1,2,3]]},null)

	console.log("%s", JSON.stringify({a:1,b:2,c:3},null,10));
	console.log("%s", JSON.stringify({a:1,b:2,c:3},["a","c"]));
	console.log("%s", JSON.stringify({a:1,b:2,c:{a:11,b:22,c:33}},["a","c"]));
	console.log("%s", JSON.stringify([1,2,3,4,5],[1,2]));
	console.log("%s", JSON.stringify([1,2,3,4,5],function(k,v){return k==""?v:v+100}));
	console.log("%s", JSON.stringify({a:1,b:2,c:{a:11,b:22,c:33}},function(k,v){
		if (k=="")
			return v;
		if (k=="a")
			return v + 100;
		else if (k == "c")
			return v;
		else
			return undefined;
	}));

	var r = 0;
	var a=[1,2,3,4];
	a[5] = a;
	try {
		console.log("%s", JSON.stringify(a));
	} catch(e) {
		if (e instanceof TypeError)
			r = 1;
	}
	if (r != 1)
		fail();

	a = {
		b:1,
		c:2
	}
	a.p = a;
	try {
		console.log("%s", JSON.stringify(a));
	} catch(e) {
		if (e instanceof TypeError)
			r = 1;
	}
	if (r != 1)
		fail();

	console.log("stringify test passed");
}

stringify_test();

function parse_test()
{
	function test(str, v) {
		var r = JSON.parse(str);
		if (r != v)
			fail();
	}

	test("true", true);
	test("false", false);
	test("null", null);
	test("\"sophon\"", "sophon");
	test("123.45", "123.45");

	var r, i;

	r = JSON.parse("[0,1,2,3,4,5,6,7,8,9]");
	if (!(r instanceof Array))
		fail();
	for (i = 0; i < 10; i++)
		if (r[i] != i)
			fail();

	r = JSON.parse("{\"a\":1,\"b\":2,\"c\":{\"a\":11,\"b\":22,\"c\":33}}");
	if (r.a != 1 || r.b != 2 || r.c.a !=11 || r.c.b != 22 || r.c.c != 33)
		fail();

	r = JSON.parse("{\"a\":1,\"b\":2,\"c\":3}", function(k,v){
		return k==""?v:v+100;
	});
	if (r.a!=101 || r.b!=102 || r.c!=103)
		fail();

	r = JSON.parse("{\"a\":1,\"b\":2,\"c\":3}", function(k,v){
		if (k=="b")
			return undefined;
		return k==""?v:v+100;
	});
	if (r.a!=101 || r.c!=103 || r.hasOwnProperty("b"))
		fail();

	console.log("parse test passed");
}

parse_test();

console.log("all tests passed");
