# UE4设计模式 - 简介

> 每一个模式描述了一个在我们周围不断重复发生的问题，以及该问题的解决方案的核心。这样，你就能一次又一次地使用该方案而不必做重复的劳动。
> —— Christopher Alexander

## 1. 前言

软件工程就是用来解决现实生活各种重复的劳动，我们通过对现实世界中重复的过程进行建模，转变为计算机可以理解的代码和指令，从而将重复繁琐的工作交由计算机来完成。软件工程可以解决其他领域中重复劳动的问题，但是在开发软件的过程中，也不可避免的会面临一些重复劳动。我们经常会遇到一些相同的问题，每次可能都会从头做起。重复的解决一个问题让我们在面对相同情景的时候会变得游刃有余，但是本身并没有提升，我们还是停留在解决问题上，并没有思考问题的核心原因。设计模式通过将软件开发过程经常遇到的重复问题进行收集分类，并剖析问题的核心原因，系统地记录其解决方案和决策过程，让我们可以透过问题看到变化的本质。**封装变化的概念**，这是许多设计模式的主题。在学习和理解设计模式的过程中，理解其中变化是比较关键的。

新手在学习设计模式的过程中，比较困难的是，由于写的代码量比较少，遇到的问题也不够多，碰巧遇到类似的问题时，可能通过复制粘贴或者冗长的分支语句来解决问题。最后代码里面充斥着大量的重复代码，导致维护起来变得日益困难，新功能的添加也变得越来越难。当他们阅读那些设计优良的代码时，由于没有经历过迭代开发，也很难理解作者为何要这样设计。整个学习过程变得困难起来。**《设计模式-可复用面向对象软件的基础》**正是解决这个问题的关键，通过提前学习和理解那些复用性和灵活性好的设计，这样当我们遇到类似问题时，可以联想到手头的问题是否有对应的设计模式可以解决，并将其应用到实际开发过程中，渐渐地积累起我们的解决方案。**不要重复你自己**，当我们觉察到自己的代码变得越来越重复的时候，记得停下来，认真思考一下，这里是否有更优雅的解决方式。

> Don't repeat yourself [[1\]](https://zhuanlan.zhihu.com/p/141561198#ref_1)

## 2. 为什么要重复讲解一遍设计模式？

因为笔者正从事UE4游戏开发，UE4是一款庞大的工业游戏引擎，所有的设计模式在UE4中都有应用，通过提炼出其中的实际例子可以更深刻的理解UE4引擎和设计模式，相对于《设计模式》书上的已知应用，所有的代码片段都来自UE4引擎实际代码。随着时间的流逝，设计模式中很多模式如今已经成为行业标准了，如 Iterator迭代器。很多书中的设计模式也逐渐变得更加灵活，如Factory Method的演化版本Auto Register Factory。分析实际的代码可以让我们整体上有一个把握，一些具体的实现细节也能够体现出来。

> God is in the detail.

整理和收集这些模式本身也是一个有趣的过程，希望能够对大家学习UE4有所帮助。

Happy Reading! ;-)

## 3. 面向对象设计的原则

- **针对接口编程，而不是针对实现编程**
- **优先使用对象组合，而不是类继承**

## 4. 设计模式所支持的设计的可变方面

![img](https://pic3.zhimg.com/80/v2-0a2c9985300441c08de5059164bf62f2_720w.webp)

图 1-1 设计模式所支持的设计的可变方面

## 5. 目录

## 创建型模式

- [AbstractFactory（抽象工厂）- 未完成](https://zhuanlan.zhihu.com/p/141542546)
- [Builder（生成器）- 未完成](https://zhuanlan.zhihu.com/p/141543218)
- [FactoryMethod（工厂方法）- 未完成](https://zhuanlan.zhihu.com/p/141543355)
- [ProtoType（原型）- 未完成](https://zhuanlan.zhihu.com/p/141543766)
- [Singleton（单件）- 未完成](https://zhuanlan.zhihu.com/p/141546105)

## 结构型模式

- [Adapter（适配器）- 未完成](https://zhuanlan.zhihu.com/p/141546260)
- [Bridge（桥接）- 未完成](https://zhuanlan.zhihu.com/p/141546382)
- [Composite（组合）- 未完成](https://zhuanlan.zhihu.com/p/141546499)
- [Decorator（装饰）- 未完成](https://zhuanlan.zhihu.com/p/141546814)
- [Facade（外观）- 未完成](https://zhuanlan.zhihu.com/p/141546889)
- [Flyweight（享元）- 未完成](https://zhuanlan.zhihu.com/p/141547055)
- [Proxy（代理）- 未完成](https://zhuanlan.zhihu.com/p/141554391)

## 行为模式

- [ChainOfResponsibility（职责链）- 未完成](https://zhuanlan.zhihu.com/p/141554491)
- [Command（命令）- 未完成](https://zhuanlan.zhihu.com/p/141554976)
- [Interpreter（解释器）- 未完成](https://zhuanlan.zhihu.com/p/141555450)
- [Iterator（迭代器）- 未完成](https://zhuanlan.zhihu.com/p/141555550)
- [Mediator（中介者）- 未完成](https://zhuanlan.zhihu.com/p/141555733)
- [Memento（备忘录）- 未完成](https://zhuanlan.zhihu.com/p/141556022)
- [Observer（观察者）- 未完成](https://zhuanlan.zhihu.com/p/141556120)
- [State（状态）- 未完成](https://zhuanlan.zhihu.com/p/141556164)
- [Strategy（策略）- 未完成](https://zhuanlan.zhihu.com/p/141556336)
- [TemplateMethod（模板方法）- 未完成](https://zhuanlan.zhihu.com/p/141560724)
- [Visitor（访问者）- 未完成](https://zhuanlan.zhihu.com/p/141560841)

## 6. 设计模式相关书籍

- 《设计模式：可复用面向对象软件的基础》
- 《设计模式沉思录》
- 《程序员修炼之道：从小工到专家》
- 《重构：改善既有代码的设计》
- 《代码整洁之道》
- 《架构整洁之道》
- 《UNIX编程艺术》
- 《C++API设计》

## 参考

1. [^](https://zhuanlan.zhihu.com/p/141561198#ref_1_0)DRY [https://en.wikipedia.org/wiki/Don%27t_repeat_yourself](https://en.wikipedia.org/wiki/Don't_repeat_yourself)