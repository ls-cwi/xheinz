TODO:

* Back-off functionality (expose in command line + maxIt)
* Enable zero-half cuts (and maybe others)
* Investigate why min cut separation is so slow (disabled for now...)
* Add sample files for small example

Done:

* Two epsilons
* Min cut determination using residual network
* NodeMatrix as `std::vector< std::list< GraphComponent::Node > >`
* Build up RHS first
* Intensive root node
* Purge cuts
* Lazy callback: only look at connected components 
* User callback: connected components + min cut
