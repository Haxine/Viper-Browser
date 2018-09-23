(function() {

    var onSubmitForm = function(e) {
        var form = e.currentTarget;
        var username = '', password = '';
        var formData = {};
        for (var i = 0; i < form.elements.length; ++i) {
            var elem = form.elements[i];

            let eType = elem.type.toLowerCase();
            if (eType == 'email' || eType == 'password' || eType == 'text') {
                formData[elem.name] = elem.value;
                
                let eName = elem.name.toLowerCase();
                if (eType == 'text' && (eName == 'username' || eName.indexOf('name') >= 0 || eName.indexOf('login') >= 0)) {
                    username = elem.value;
                } else if (eType == 'email' && username == '') {
                    username = elem.value;
                } else if (eType == 'password') {
                    password = elem.value;
                }
            }
        }

        if (password.length > 0) {
            window.viper.autofill.onFormSubmitted(window.location.href, username, password, formData);
        }
    };

    for (let form of document.forms) {
        form.addEventListener('submit', onSubmitForm);
        //todo: check for a username input field, add event listener for the change event,
        //      and call autofill to see if the username that has been entered has an
        //      associated password stored for it
    }

    var options = {
        childList: true,
        subtree: true
    };

    var mutCallback = function(mutationList, observer) {
        for (let mut of mutationList) {
            for (let node of mut.addedNodes) {
                if (node.nodeName.toLowerCase() == 'form') {
                    node.addEventListener('submit', onSubmitForm);
                }
            }
        }
    };

    var observer = new MutationObserver(mutCallback);
    observer.observe(document.documentElement, options);

})();
