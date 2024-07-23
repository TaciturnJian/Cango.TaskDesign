# Cango.TaskDesign 面向任务设计的代码框架

此框架讨论并提供如下问题的一些解决方案：

- [任务中重要角色的生命周期问题](#LifeTimeOwnership)
- 任务的中断、是否完成等信号状态问题 DoneSignal
- 拥有自己独立上下文执行任务的角色 ExecutableTask
- 传递任务中，货物对象的的产生和接收问题 ItemDelivery
- 并行任务的管理问题 JoinThreads


## LifeTimeOwnership

使用 std::shared_ptr 配合 std::weak_ptr 管理对象的生命周期。

利用类型别名来确定变量是否管理对象的生命周期。

我们将对象的生命周期分为四个部分：创建、存活(配置、使用)、销毁。  
使用 std::shared_ptr 管理对象的创建、存活和销毁。  
使用 std::weak_ptr 和一些语义来区分对象的配置与使用。

假设有一个工厂存在，那么这个工厂负责 ObjectOwner 的创建和配置，之后这个货物有下面三种去向：

1. 移交货物，那么货物就在用户上下文中成为了 ObjectOnwer ，拥有货物的同时也负责货物的生命周期，工厂不再负责货物。
2. 使用货物，那么货物就在用户上下文中成为了 ObjectUser ，使用货物的同时，生命周期由工厂和当前上下文共同管理。
3. 获取凭证，那么货物就在用户上下文中成为了 Credential ，不负责货物的生命周期，只负责获取货物的使用权，生命周期由工厂管理。

使用凭证，可以把货物转换成 ObjectUser ，在上下文中使用货物，生命周期由当前上下文和工厂还有其他使用者共同管理。

