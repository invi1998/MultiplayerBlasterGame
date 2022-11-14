 将仰角数据从 [270, 360) 映射到 [-90, 0)
		 这个颜色将会修复因为在 CharacterMovementComponent中，这个组件在GetPackedAngles（获取仰角数据）
		 这个函数为了将仰角数据（旋转数据）通过RPC网络传输时，为了减少宽带占用，将数据压缩到4字节，
		 导致我们客户端的俯视角在服务端显示确实抬头的仰视角的bug
		 在我们的程序编写中，我们习惯的角度控制是例如 -90到90，但是在虚幻引擎中，这个函数在获取这些旋转角度数据时，
		 他会将其压缩为一个无符号的整形数
		 这个压缩过程就是通过 FRotator::CompressAxisToShort(Yaw) , FRotator::CompressAxisToShort(Patch)
		 这两个函数进行的压缩处理，函数原型如下

```c++
FORCEINLINE uint32 UCharacterMovementComponent::PackYawAndPitchTo32(const float Yaw, const float Pitch)
{
	const uint32 YawShort = FRotator::CompressAxisToShort(Yaw);
	const uint32 PitchShort = FRotator::CompressAxisToShort(Pitch);
	const uint32 Rotation32 = (YawShort << 16) | PitchShort;
	return Rotation32;
}
```



​		 然后关于 这里主要的执行函数 CompressAxisToShort,原型如下
```c++
		template<typename T>
		FORCEINLINE uint16 TRotator<T>::CompressAxisToShort(T Angle)
		{
			// map [0->360) to [0->65536) and mask off any winding
			return FMath::RoundToInt(Angle * (T)65536.f / (T)360.f) & 0xFFFF;
		}
```

​		 从源码中可以看出来，这个函数这里所做的，就是将一个float的角度数据，他的数值范围是[0,360)，
​		 (Angle * (T)65536.f / (T)360.f)这串计算得到的结果是一个介于 [0, 65536)之间的浮点值
​		 RoundToInt则是进行四舍五入，将浮点数转为整形
​		 我们知道float数据是具有小数点精度的，这些精度信息是需要字节信息来进行存储的，
​		 通过将float数据mask映射到这个范围，并删除这些小数点，我们就得到了一个一个介于 [0, 65536)之间的整形值
​		 而 65535 这个值他的十六进制数正好是 0xFFFF,16bit位的最大整形数
​		 将前面的整形数与 0xFFFF 进行按位与 & 运算，得到的结果就是将传递来的浮点数按360°进行切割。
​		 意思就是，假设你传递进来361，那么在经过计算后，得到的结果其实就是1，这一轮的旋转其实和你旋转1°是一样的效果
​		 这个就是虚幻底层在调用RPC传输旋转角和俯仰角时，为了减少网络开销所做的优化