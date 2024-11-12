function hook_java() {
    Java.perform(function() {
        var process_clz = Java.use("android.os.Process");
        process_clz.killProcess.implementation = function(pid) {
            console.log("kill process : ",pid);
        }

        var main_clz = Java.use("com.gdufs.xman.MyApp");
        main_clz.saveSN.implementation = function(str) {
            this.saveSN(str);
            console.log("MyApp : ",str);
        }
    })
}

function hook_native() {
    var lib_hello = Module.findBaseAddress("libmyjni.so");
    if(lib_hello != NULL) {
        Interceptor.attach(lib_hello.add(0x13B0),{
            onEnter:function(args) {
                console.log("libmyjni initSN");
            },onLeave:function(ret) {

            }
        });
    }
}

function main() {
    hook_java();
    hook_native();
}

setImmediate(main)