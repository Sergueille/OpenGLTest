const summary = document.getElementById("summary");
const titles = document.querySelectorAll("h3");

for (let title of titles)
{
    let listItem = document.createElement("li");
    let link = document.createElement("a");
    link.innerHTML = title.innerHTML;
    link.setAttribute("href", "#" + title.id)

    listItem.appendChild(link);
    summary.appendChild(listItem);
}
