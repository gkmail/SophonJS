function fail()
{
	throw Error("test failed");
}

function to_int32_test()
{
	var r;

	r = 2147483647>>0;
	if (r != 2147483647)
		fail();

	r = 2415919103>>0;
	if (r != -1879048193)
		fail();

	r = 4294967296>>0;
	if (r != 0)
		fail();

	r = 14294967296>>0;
	if (r != 1410065408)
		fail();

	r = 268435455>>0;
	if (r != 268435455)
		fail();

	r = -2147483647>>0;
	if (r != -2147483647)
		fail();

	r = -2415919103>>0;
	if (r != 1879048193)
		fail();

	r = -4294967296>>0;
	if (r != 0)
		fail();

	r = -14294967296>>0;
	if (r != -1410065408)
		fail();

	r = -268435455>>0;
	if (r != -268435455)
		fail();

	console.log("to int32 test passed");
}

to_int32_test();

function to_uint32_test()
{
	var r;

	r = 2147483647>>>0;
	if (r != 2147483647)
		fail();

	r = 2415919103>>>0;
	if (r != 2415919103)
		fail();

	r = 4294967296>>>0;
	if (r != 0)
		fail();

	r = 14294967296>>>0;
	if (r != 1410065408)
		fail();

	r = 268435455>>>0;
	if (r != 268435455)
		fail();

	r = -2147483647>>>0;
	if (r != 2147483649)
		fail();

	r = -2415919103>>>0;
	if (r != 1879048193)
		fail();

	r = -4294967296>>>0;
	if (r != 0)
		fail();

	r = -14294967296>>>0;
	if (r != 2884901888)
		fail();

	r = -268435455>>>0;
	if (r != 4026531841)
		fail();

	console.log("to uint32 test passed");
}

to_uint32_test();

function shift_test()
{
	var r;

	r = 0xFFFFFFFF >> 0x21;
	if (r != -1)
		fail();

	r = 0xFFFFFFFF >>> 0x20;
	if (r != 4294967295)
		fail();

	r = 0xFFFFFFFF >>> 0x21;
	if (r != 2147483647)
		fail();

	r = 0x80000000 >> 1;
	if (r != -1073741824)
		fail();

	r = 0x80000000 >> 2;
	if (r != -536870912)
		fail();

	r = 0x7FFFFFFF >> 1;
	if (r != 1073741823)
		fail();

	r = 0x7FFFFFFF >> 2;
	if (r != 536870911)
		fail();

	r = 0xFFFFFFFF << 1;
	if (r != -2)
		fail();

	r = 0xFFFFFFFF << 2;
	if (r != -4)
		fail();

	r = 0x80000000 << 1;
	if (r != 0)
		fail();

	console.log("shift test passed");
}

shift_test();

function expr_test()
{
	var r;

	r = -0;
	if (r != 0)
		fail();

	r = -19781009;
	if (r != -19781009)
		fail();

	r = +0;
	if (r != 0)
		fail();

	r = +18767651;
	if (r != 18767651)
		fail();

	r = 23*64+99/3;
	if (r != 1505)
		fail();

	r = void (1+2+3+4);
	if (r != void 0)
		fail();

	r = 1/0;
	if (r != Infinity)
		fail();

	r = 19781009%87;
	if (r != 80)
		fail();

	r = 10%0;
	if (!isNaN(r))
		fail();

	r = 1+"23";
	if (r != "123")
		fail();

	r = 3*"33";
	if (r != 99)
		fail();

	r = "head"+"tail";
	if (r != "headtail")
		fail()

	r = 0x7FFFFFFF*2;
	if (r != 4294967294)
		fail();

	r = 0x7FFFFFFF*4;
	if (r != 8589934588)
		fail();

	r = 1 << 10;
	if (r != 1024)
		fail();

	r = 1024 >> 10;
	if (r != 1)
		fail();

	r = -1024 >> 10;
	if (r != -1)
		fail();

	r = -1024 >>> 10;
	if (r != 4194303)
		fail();

	r = 0 ^ 0;
	if (r != 0)
		fail();

	r = 0 ^ 1;
	if (r != 1)
		fail();

	r = 0xFFFF0000 ^ 0xFFFFFFFF;
	if (r != 0xFFFF)
		fail();

	r = ~1;
	if (r != -2)
		fail();

	r = ~0;
	if (r != -1)
		fail();

	r=0,1,2,3;
	if (r != 0)
		fail();

	r=(0,1,2,3);
	if (r != 3)
		fail();

	0,r=1,2,3;
	if (r != 1)
		fail();

	console.log("expr test passed");
}

expr_test();

function typeof_test()
{
	function test (v, r)
	{
		if (typeof v != r)
			fail();
	}

	test(void 0, "undefined");
	test(null, "object");
	test(true, "boolean");
	test(false, "boolean");
	test(1, "number");
	test("", "string");
	test(function(){}, "function");
	test(new Object, "object");

	console.log("typeof test passed");
}

typeof_test();

function compare_test()
{
	function test (v)
	{
		if (!v)
			fail();
	}

	test(100<101);
	test(100<"0x100");
	test("100">"0x100");
	test(255>=0xff);
	test(255<=0xff);
	test("255">"0xff");
	test(!(NaN > 1));
	test(!(NaN < 1));

	console.log("compare test passed");
}

compare_test();

function instanceof_test()
{
	function test (o, c)
	{
		if (!(o instanceof c))
			fail();
	}

	function test_not (o, c)
	{
		if (o instanceof c)
			fail();
	}

	function constructor ()
	{
	}

	function constructor2 ()
	{
	}

	function func()
	{
	}

	test(new String(), String);
	test(new String(), Object);
	test_not("", String);
	test(new Number(), Number);
	test(new Number(), Object);
	test_not(1, Number);
	test(new Boolean(), Boolean);
	test(new Boolean(), Object);
	test_not(true, Boolean);
	test(func, Function);
	test(func, Object);
	test(new constructor(), constructor);
	test_not(new constructor2(), constructor);

	console.log("instanceof test passed");
}

instanceof_test();

function in_test()
{
	function test (p, o)
	{
		if (!(p in o))
			fail();
	}

	function test_not (p, o)
	{
		if (p in o)
			fail();
	}

	test("p", {p:1});
	test_not("p", {pp:1});
	test(1, [0,1]);
	test_not(1, [0]);
	test_not(1, [0,,2]);

	console.log("in test passed");
}

in_test();

function equal_test()
{
	function test(e)
	{
		if (!e)
			fail();
	}

	function constructor()
	{
	}

	test(null==void 0);
	test(null==null);
	test(void 0==void 0);
	test(NaN!=NaN);
	test(-3.1415926==-3.1415926);
	test(true==!false);
	test("headtail"=="head"+"tail");
	test(255=="0xff");
	test("0x80"==128);
	test("255"!="0xff");
	test(""+255==""+0xff);
	test(+"255"==+"0xff");
	test(!"255"==!"xxx");
	test(!""==!false);
	test("0x80"!==128);
	test(new constructor()!=new constructor());
	var a = new constructor();
	b = a;
	test(a==b);

	console.log("equal test passed");
}

equal_test();

function bits_op_test()
{
	function test(e)
	{
		if (!e)
			fail();
	}

	test((0&0)==0);
	test((0&0xFFFFFFFF)==0);
	test((0xFFFF&0xFFFF0000)==0);
	test((0xFFFFFFFF&0xFFFF0000)==-65536);
	test((0|0)==0);
	test((0|0xFFFFFFFF)==-1);
	test((0xFFFF|0xFFFF0000)==-1);
	test((0xFFFFFFFF|0xFFFF0000)==-1);
	test((0^0)==0);
	test((0^0xFFFFFFFF)==-1);
	test((0xFFFF^0xFFFF0000)==-1);
	test((0xFFFF^0xFFFFFFFF)==-65536);

	console.log("bits op test passed");
}

bits_op_test();

function logic_op_test()
{
	function test(e, r)
	{
		if (e != r)
			fail();
	}

	test(true || true, true);
	test(true || false, true);
	test(false || true, true);
	test(false || false, false);
	test(true && true, true);
	test(true && false, false);
	test(false && true, false);
	test(false && false, false);

	var r;

	r = "xxx" || "yyy";
	if (r != "xxx")
		fail();

	r = "" || "yyy";
	if (r != "yyy")
		fail();

	r = "xxx" && "yyy";
	if (r != "yyy")
		fail();

	r = "" && "yyy";
	if (r != "")
		fail();

	console.log("logic op test passed");
}

logic_op_test();

function cond_expr_test()
{
	var r;

	r = true ? "xxx" : "yyy";
	if (r != "xxx")
		fail();

	r = false ? "xxx" : "yyy";
	if (r != "yyy")
		fail();

	r = false ? "xxx" : false ? "yyy" : "zzz";
	if (r != "zzz")
		fail();

	console.log("cond expr test passed");
}

cond_expr_test();

function assignment_test()
{
	var a, b, c;

	a=b=c=1978;
	if ((a!=1978) || (b!=1978) || (c!=1978))
		fail();

	a=b+++4;
	if ((a!=1982) || (b!=1979))
		fail();

	a=b=c=1978;
	a=++b+4;
	if ((a!=1983) || (b!=1979))
		fail();

	a=1978;
	a+=1000;
	if (a!=2978)
		fail();

	a-=1000;
	if (a!=1978)
		fail();

	a*=1000;
	if (a!=1978000)
		fail();

	a/=1000;
	if (a!=1978)
		fail();

	a%=1000;
	if (a!=978)
		fail();

	a=0xFFFF00;
	a|=0xF0;
	if (a!=0xFFFFF0)
		fail();

	a=0xFFFF00;
	a&=0xFFF0FF;
	if (a!=0xFFF000)
		fail();

	a=0xFFF000;
	a^=0xFF0000;
	if (a!=0xF000)
		fail();

	a=1;
	b=2;
	for (var i=1; i<31; i++) {
		a<<=1;

		if (a != b)
			fail();
		b *= 2;
	}

	b/=2;
	for (i=1; i<31; i++) {
		if (a!=b)
			fail();
		a>>>=1;
		b /= 2;
	}

	console.log("assignment test passed");
}

assignment_test();

function var_decl_test()
{
	var x=19781009;
	var y=19491001,z=18977213,a,b="hello";
	var c=null,d=3.1415926;
	var e=1+2+3+4;

	if (x != 19781009)
		fail();
	if (y != 19491001)
		fail();
	if (z != 18977213)
		fail();
	if (a != void 0)
		fail();
	if (b != "hello")
		fail();
	if (c != null)
		fail();
	if (d != 3.1415926)
		fail();
	if (e != 10)
		fail();

	console.log("var decl test passed");
}

var_decl_test();

function if_test()
{
	var r;

	if (0)
		fail();

	if (1) {
	} else {
		fail();
	}

	if (0) {
		fail();
	} else if (1) {
	} else {
		fail();
	}

	if (0) {
		fail();
	} else if (0) {
		fail();
	} else {
	}

	console.log("if test passed");
}

if_test();

function loop_test()
{
	var a, r;
	
	a=0;
	while(a<100){
		a++;
	}
	if (a!=100)
		fail();

	a=100;
	while(a<100){
		fail();
	}

	a=0;
	while(a++<100);
	if (a!=101)
		fail();

	a=0;
	while(a<100){
		if (a==50)
			break;
		a++;
	}
	if (a!=50)
		fail();

	a=0;
	r=0;
	while(a<100){
		if (a++%2==0)
			continue;
		r++;
	}
	if(r!=50)
		fail();

	a=0;
	do{
		a++;
	}while(a<100);
	if (a!=100)
		fail();

	a=200;
	do{
		a++;
	}while(a<100);
	if (a!=201)
		fail();

	a=0;
	do{
		a++;
		if (a==66)
			break;
	}while(a<100);
	if (a!=66)
		fail();

	a=0;
	r=0;
	do{
		a++;
		if (a%3!=0)
			continue;
		r++;
	}while(a<100);
	if (r != 33)
		fail();

	a=0;
	for(;;){
		a++;
		if (a==100)
			break;
	}
	if (a!=100)
		fail();

	for(a=0;;){
		a++;
		if(a==300)
			break;
	}
	if (a!=300)
		fail();

	for(a=0;a<300;){
		a++;
	}
	if(a!=300)
		fail();

	for(a=0;a<300;a++);
	if (a!=300)
		fail();

	a=0;
	for(;a<300;a++);
	if (a!=300)
		fail();

	a=0;
	for(;;a++){
		if (a==300)
			break;
	}
	if (a!=300)
		fail();


	for(r=0,a=0;a<300;a++){
		if (a%3!=0)
			continue;
		r++;
	}
	if (r!=100)
		fail();

	console.log("loop test passed");
}

loop_test();

function for_in_test()
{
	var a, b, r, v, i;

	a={
		p1:1,p2:2,p3:3,p4:4
	}

	r = 0;
	for (v in a) {
		r += a[v];
	}
	if (r != 10)
		fail();

	r = 0;
	for (v in a) {
		if (a[v]==3) {
			r = 1;
			break;
		}
	}
	if (r != 1)
		fail();

	r = 0;
	for (v in a) {
		if (a[v]%2)
			continue;
		r += a[v];
	}
	if (r != 6)
		fail();

	a=[1,2,3,4,5,6,7,8,9,10];

	r = 0;
	for (v in a) {
		r += a[v];
	}
	if (r != 55)
		fail();

	r = 0;
	for (v in a) {
		if (v == 10)
			break;
		r += v;
	}
	if (r != 45)
		fail();

	r = 0;
	for (v in a) {
		if (a[v] % 2)
			continue;
		r += a[v];
	}
	if (r != 30)
		fail();

	a = {p1:{
			p1:1, p2:2, p3:3
		},
		p2:{
			p1:4, p2:5, p3:6
		},
		p3:{
			p1:7, p2:8, p3:9
		}
	};
	r = 0;
	for (v in a) {
		for (i in a[v]) {
			r += a[v][i];
		}
	}
	if (r != 45)
		fail();

	a=[[1,2,3,4],[5,6,7,8],[9,10,11,12]];
	r = 0;
outer: for (v in a) {
		for (i in a[v]) {
			if (a[v][i] % 2 == 0)
				continue outer;
			r += a[v][i];
		}
	}
	if (r != 15)
		fail();

	r = 0;
outer: for (v in a) {
		for (i in a[v]) {
			if (a[v][i] % 2 == 0)
				break outer;
			r += a[v][i];
		}
	}
	if (r != 1)
		fail();

	console.log("for in test passed");
}

for_in_test();

function switch_test()
{
	var r;

	r = 0;
	switch("test") {
		case "test":
			r = 1;
			break;
		case "test2":
		default:
			fail();
			break;
	}
	if (r != 1)
		fail();

	r = 0;
	switch("test") {
		case "test":
			r = 1;
		case "test2":
			r = 2;
			break;
		default:
			fail();
			break;
	}
	if (r != 2)
		fail();

	r = 0;
	switch("testdef") {
		case "test":
		case "test2":
			break;
		default:
			r = 3;
			break;
	}
	if (r != 3)
		fail();

	r = 0;
	switch("testdef") {
		default:
			r = 3;
			break;
		case "test":
		case "test2":
			fail();
			break;
	}
	if (r != 3)
		fail();

	r = 0;
	switch("testdef") {
		default:
			r++;
		case "test":
			r++;
			break;
		case "test2":
			fail();
			break;
	}
	if (r != 2)
		fail();

	r = 0;
	switch("testdef") {
		case "1":
		case "2":
			fail();
			break;
		default:
			r++;
		case "test":
			r++;
			break;
		case "test2":
			fail();
			break;
	}
	if (r != 2)
		fail();

	console.log("switch test passed");
}

switch_test();

function return_test()
{
	var r;

	function test1(){return;}
	r = test1();
	if (r != void 0)
		fail();

	function test2(){return 19781009;}
	r = test2();
	if (r != 19781009)
		fail();

	console.log("return test passed");
}

return_test();

function try_test()
{
	var r, a, b;

	r = 0;
	try{
		a = 1;
		b = 1;
		r = 1;
	}catch(e){
		r = 2;
	}
	if (r != 1)
		fail();

	r = 0;
	try{
		a = 1;
		b = 1;
		r = 1;
		throw Error("test");
	}catch(e){
		if ((e instanceof Error) && (e.message == "test"))
			r = 2;
	}
	if (r != 2)
		fail();

	r = 0;
	try{
		a = 1;
		b = 1;
		r = 1;
	}finally{
		r = 2;
	}
	if (r != 2)
		fail();

	r = 0;
	try{
		a = 1;
		b = 1;
		r = 1;
		throw Error("excep");
	}catch(e){
		if ((e instanceof Error) && (e.message == "excep"))
			r += 10;
	}finally{
		r += 100;
	}
	if (r != 111)
		fail();

	var e=19781009;
	if (e != 19781009)
		fail();
	r = 0;
	try{
		throw Error("test");
	}catch(e){
		if (!(e instanceof Error) || (e.message != "test"))
			fail();

		function get_error(){
			return e;
		}

		r = get_error();
	}
	if (e != 19781009)
		fail();
	if (!(r instanceof Error) || (r.message != "test"))
		fail();

	console.log("try test passed");
}

try_test();

function with_test()
{
	var a, b, c, r;

	a=b=c=1;

	function func(){
		return 1978;
	}

	a={
		b:"hello",
		c:"world",
		func:function(){
			return b+c;
		}
	}

	if (b!=1)
		fail();
	if (c!=1)
		fail();
	if (func()!=1978)
		fail();

	with(a){
		if (b!="hello")
			fail();
		if (c!="world")
			fail();
		if (func()!=2)
			fail();

		function get_b(){
			return b;
		}

		r = get_b();
	}

	if (b!=1)
		fail();
	if (c!=1)
		fail();
	if (func()!=1978)
		fail();
	if (r != "hello")
		fail();

	console.log("with test passed");
}

with_test();

var x = 19781009;

function delete_test()
{
	var r;

	o = Object();
	o.p = 1;
	if (!o.hasOwnProperty("p"))
		fail();
	r = delete o.p;
	if (r != true)
		fail();
	if (o.hasOwnProperty("p"))
		fail();

	if (x != 19781009)
		fail();
	eval("var x=\"sophon\"");
	if (x != "sophon")
		fail();
	r = delete x;
	if (r != true)
		fail();
	if (x != 19781009)
		fail();

	console.log("delete test passed");
}

delete_test();

console.log("all tests passed");
