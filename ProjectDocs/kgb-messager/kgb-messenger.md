# kgb-messager

kgb-messager是CTF的一道题，这里记录一下解题过程，练练手。

案例地址：https://github.com/Soon-gz/blog/tree/main/ProjectDocs/kgb-messager

## 启动闪退

启动闪退，提示Integrity Error，使用jadx打开apk，搜索字符串Integrity Error，定位到mainActivity源码

![1](/images/1.png)

```
@Override // android.support.v7.app.c, android.support.v4.b.l, android.support.v4.b.h, android.app.Activity
    public void onCreate(Bundle bundle) {
        super.onCreate(bundle);
        setContentView(R.layout.activity_main);
        String property = System.getProperty("user.home");
        String str = System.getenv("USER");
        if (property == null || property.isEmpty() || !property.equals("Russia")) {
            a("Integrity Error", "This app can only run on Russian devices.");
        } else if (str == null || str.isEmpty() || !str.equals(getResources().getString(R.string.User))) {
            a("Integrity Error", "Must be on the user whitelist.");
        } else {
            a.a(this);
            startActivity(new Intent(this, LoginActivity.class));
        }
    }

```

可以看到getProperty需要得到的值是Russia，查看 R.string.User的值是RkxBR3s1N0VSTDFOR180UkNIM1J9Cg==，那么写frida的js

```
function hook() {
    Java.perform(function() {
        var System = Java.use("java.lang.System");
        System.getProperty.overload("java.lang.String").implementation = function(arg) {
            return "Russia";
        }
        System.getenv.overload("java.lang.String").implementation = function(arg) {
            return "RkxBR3s1N0VSTDFOR180UkNIM1J9Cg==";
        }
    })
}
```

hook java的System getProperty函数，强制返回Russia，并hook getenv，强制返回RkxBR3s1N0VSTDFOR180UkNIM1J9Cg==，可以到登录页面。

### 第一个flag

RkxBR3s1N0VSTDFOR180UkNIM1J9Cg==解Base64后，这里有了第一个flag ： FLAG{57ERL1NG_4RCH3R}

## 登录

![2](\images\2.png)

可以看到登录源码

```
public void onLogin(View view) {
        EditText editText = (EditText) findViewById(R.id.login_username);
        EditText editText2 = (EditText) findViewById(R.id.login_password);
        this.n = editText.getText().toString();
        this.o = editText2.getText().toString();
        if (this.n == null || this.o == null || this.n.isEmpty() || this.o.isEmpty()) {
            return;
        }
        if (!this.n.equals(getResources().getString(R.string.username))) {
            Toast.makeText(this, "User not recognized.", 0).show();
            editText.setText("");
            editText2.setText("");
        } else if (j()) {
            i();
            startActivity(new Intent(this, MessengerActivity.class));
        } else {
            Toast.makeText(this, "Incorrect password.", 0).show();
            editText.setText("");
            editText2.setText("");
        }
    }
```

查看R.string.username的值为codenameduchess，password的值为84e343a0486ff05530df6c705c8bb4。这里的值只有30位，而标准的md5应该是32位，所以应该是0084e343a0486ff05530df6c705c8bb4，上md解密网站，https://www.somd5.com/

![3](\images\3.png)

所以用户名和密码就有了，用户名codenameduchess，密码 guset，输入后即可登录成功。

### 第二个flag

登录成后，会toast提示一个flag，FLAG{G00G13_PR0}

## 聊天页面

![4](\images\4.png)

这里随意输入文本，没有任何响应，查看java源码MessengerActivity的onSendMessage

```
public void onSendMessage(View view) {
        EditText editText = (EditText) findViewById(R.id.edittext_chatbox);
        String obj = editText.getText().toString();
        if (TextUtils.isEmpty(obj)) {
            return;
        }
        this.o.add(new com.tlamb96.kgbmessenger.b.a(R.string.user, obj, j(), false));
        this.n.c();
        if (a(obj.toString()).equals(this.p)) {
            Log.d("MessengerActivity", "Successfully asked Boris for the password.");
            this.q = obj.toString();
            this.o.add(new com.tlamb96.kgbmessenger.b.a(R.string.boris, "Only if you ask nicely", j(), true));
            this.n.c();
        }
        if (b(obj.toString()).equals(this.r)) {
            Log.d("MessengerActivity", "Successfully asked Boris nicely for the password.");
            this.s = obj.toString();
            this.o.add(new com.tlamb96.kgbmessenger.b.a(R.string.boris, "Wow, no one has ever been so nice to me! Here you go friend: FLAG{" + i() + "}", j(), true));
            this.n.c();
        }
        this.m.b(this.m.getAdapter().a() - 1);
        editText.setText("");
    }
```

这里输入的字符串和p字段要经过a函数进行判断处理，a函数源码如下

```
    private String a(String str) {
        char[] charArray = str.toCharArray();
        for (int i = 0; i < charArray.length / 2; i++) {
            char c = charArray[i];
            charArray[i] = (char) (charArray[(charArray.length - i) - 1] ^ '2');
            charArray[(charArray.length - i) - 1] = (char) (c ^ 'A');
        }
        return new String(charArray);
    }
```

```
private String p = "V@]EAASB\u0012WZF\u0012e,a$7(&am2(3.\u0003";
```

那么可以根据p的字符串逆推得到加密前的字符串。

```
def main():
    p = "V@]EAASB\u0012WZF\u0012e,a$7(&am2(3.\u0003"
    p_result = bytearray(p.encode())
    for i in range(int(len(p_result)/2)):
        c = p_result[i]
        p_result[i] = (p_result[(len(p_result)- i) - 1] ^ ord('A'))
        p_result[(len(p_result)- i) - 1] = c ^ ord('2')

    print(p_result)
    flag = ""
    for i in range(int(len(p_result))):
        flag += chr(p_result[i])

    print(flag)

if __name__ == "__main__":
    main()
```

使用python脚本进行解密，java代码的加密过程是前半字符=后半字符异或'2'，后半字符=前半字符异或'A'，那么解密的时候逆推即可。

得到原字符串：“Boris, give me the password”

![5](\images\5.png)

onSendMessage里面还有一个r字段的判断。调用了b函数

```
    private String b(String str) {
        char[] charArray = str.toCharArray();
        for (int i = 0; i < charArray.length; i++) {
            charArray[i] = (char) ((charArray[i] >> (i % 8)) ^ charArray[i]);
        }
        for (int i2 = 0; i2 < charArray.length / 2; i2++) {
            char c = charArray[i2];
            charArray[i2] = charArray[(charArray.length - i2) - 1];
            charArray[(charArray.length - i2) - 1] = c;
        }
        return new String(charArray);
    }

```

```
private String r = "\u0000dslp}oQ\u0000 dks$|M\u0000h +AYQg\u0000P*!M$gQ\u0000";
```

可以通过这个查看加密算法，来逆推源字符串，这个稍微复杂一点，可以使用python Z3库。

```
from z3 import *

def main():
    s = Solver()
    r = "\u0000dslp}oQ\u0000 dks$|M\u0000h +AYQg\u0000P*!M$gQ\u0000"
    r_result = bytearray(r.encode())
    for i in range(int(len(r_result)/2)):
        c = r_result[i]
        r_result[i]=r_result[(len(r_result) - i) - 1]
        r_result[(len(r_result) - i) - 1]=c
    z = [BitVec("z%s" % i,32) for i in range(len(r_result))]
    for i in range(len(r_result)):
        s.add(r_result[i] == (z[i] >> (i % 8) ^ z[i]))
    if(s.check() == sat):
        model = s.model()
        print(model)
        flag=""
        for i in range(len(r_result)):
            if(model[z[i]] != None):
                chr_val = model[z[i]].as_long().real
                if chr_val > 255:
                    chr_val = (~chr_val)&0xff
                flag += chr(chr_val)
            else:
                flag += " "
        print('"'+flag+'"')



if __name__ == "__main__":
    main()
```

java的算法先将前后数据交换了，python解密时需要先交换回来。

z3 solver 知道结果，知道方程式，那么可以按照每个字符的加密方式进行设置，直接得出解。

加密方程式：r_result[i] == (z[i] >> (i % 8) ^ z[i])，其中r_result是我们已知的结果，等式后面是加密算法。

得到结果：

" ay I *P EASE* h ve the  assword "

![6](\images\6.png)

### 第三个flag

通过聊天解密得到第三个flag ： FLAG{p455w0rd_P134SE}