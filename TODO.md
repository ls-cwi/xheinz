Changes:

* Lazy callback: only look at connected components 
* User callback: connected components + min cut
* Build up RHS first
* Back-off functionality
* Purge cuts
* Intensive root node
* Enable zero-half cuts (and maybe otherS)

Done:

* Two epsilons
* Min cut determination using residual network
* NodeMatrix as `std::vector< std::list< GraphComponent::Node > >`
