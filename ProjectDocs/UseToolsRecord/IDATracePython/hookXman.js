
function hookRegister() {
    var libart = Process.getModuleByName("libart.so");
    var symbals = libart.enumerateSymbols();
    for (var i = 0; i < symbals.length; i++) {
        var symbal = symbals[i];
        if(symbal.name.indexOf("_ZN3art3JNI15RegisterNatives") != -1) {
            console.log("RegisterNatives : "+symbal.name);
            Interceptor.attach(symbal.address,{
                onEnter:function(args) {
                    var number = args[3];
                    console.log("RegisterNatives number: "+number);
                    var methods = args[2];
                    for (var i = 0; i < number; i++) {
                        var methodname = ptr(methods).add(Process.pointerSize * 3 * i).readPointer().readCString();
                        var sig = ptr(methods).add(Process.pointerSize * 3 * i).add(Process.pointerSize).readPointer().readCString();
                        var addr = ptr(methods).add(Process.pointerSize * 3 * i).add(Process.pointerSize*2);
                        console.log("RegisterNative:name->" + methodname + ",  sig:" + sig + "   addr:" + addr);
                    }
                },onLeave:function(retval){}
            })
        }
    }

    var libc = Process.getModuleByName("libc.so");
    var strstr = libc.getExportByName("strcmp");
    var fopen = libc.getExportByName("fopen");
    Interceptor.attach(strstr,{
        onEnter:function(args) {
            var dat = ptr(args[1]).readCString();
            if(dat.indexOf("EoPAoY62@ElRD") != -1) {
                var arg1 = ptr(args[0]).readCString();
                console.log("arg0 -> " +arg1+"  strcmp  arg1 -> "+dat);
            }
        },onLeave:function(retval){}
    });
    Interceptor.attach(fopen,{
        onEnter:function(args) {
            var dat = ptr(args[0]).readCString();
            if(dat.indexOf("sdcard") != -1) {
                var arg1 = ptr(args[1]).readCString();
                console.log("arg0 -> " +dat+"  fopen  arg1 -> "+arg1);
            }
        },onLeave:function(retval){
            console.log("ret val -->"+retval);
        }
    });
}

function saveDatLibc() {
    var libc = Process.getModuleByName("libc.so");
    var fopen_addr = libc.getExportByName("fopen");
    var fclose_addr = libc.getExportByName("fclose");
    var fputs_addr = libc.getExportByName("fputs");
    var fopen_func = new NativeFunction(fopen_addr,"pointer",["pointer","pointer"]);
    var fclose_func = new NativeFunction(fclose_addr,"int",["pointer"]);
    var fputs_func = new NativeFunction(fputs_addr,"int",["pointer","pointer"]);
    var path = Memory.allocUtf8String("sdcard/reg.dat");
    var r = Memory.allocUtf8String("w+");
    var fd = fopen_func(path,r);
    var dat = Memory.allocUtf8String("EoPAoY62@ElRD");
    fputs_func(dat,fd);
    fclose_func(fd);
    
}

function main() {
    hookRegister();
}

setImmediate(main)