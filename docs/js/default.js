$(document).ready(function () {
    // if sessionStorage has tree-of-files data:
    //     use it, because it has state info about the tree
    // else
    //     leave the original tree-of-files in place
    var data = window.sessionStorage.getItem("tree-of-files");
    if (data) {
        $('#tree-of-files').html(data);
    } else {
        $('.tree-of-files-children').removeClass("show").addClass("dontshow");
    }


    // Allow user to click section headings to show/hide their children.
    $('.tree-of-files-dir').click(function () {
        var triangle = $(this).children('.ui-icon');
        var children = $(this).next('.tree-of-files-children');
        if (children.hasClass('show')) {
            triangle.removeClass("ui-icon-triangle-1-s").addClass("ui-icon-triangle-1-e");
            children.removeClass("show").addClass("dontshow");
        } else if (children.hasClass('dontshow')) {
            triangle.removeClass("ui-icon-triangle-1-e").addClass("ui-icon-triangle-1-s");
            children.removeClass("dontshow").addClass("show");
        }
    });


    // Save tree-of-files to session storage.
    $('.save-tree').click(function () {
        var data = $('#tree-of-files').html();
        window.sessionStorage.setItem("tree-of-files", data);
    });
});
