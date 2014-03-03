Changes:

* Lazy callback: only look at connected components (NodeMatrix as std::vector< std::list< GraphComponent::Node > >)
* User callback: connected components + min cut
* Build up RHS first
* Back-off functionality
* Purge cuts
* Intensive root node

Done:

* Two epsilons
* Min cut determination using residual network

