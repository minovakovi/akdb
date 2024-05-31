'use strict';

let sections = {};
let i = 0;

let isMobileMenuToggled = false;
let isMobile = false;

window.addEventListener('resize', () => {
    isMobile = window.innerWidth < 1080;

    if (!isMobile) {
        document.querySelector('nav ul').style.setProperty('height', '2rem');
    } else {
        correctMobileScroll();

        document.querySelector('nav ul').style.setProperty('height', '0');
        isMobileMenuToggled = false;
        setMobileMenuBtnIcon();
    }

    refreshScrollspyItems();

});

window.addEventListener('scroll', () => {
    let condition = window.scrollY > 60;

    toggleClass(document.querySelector('#scroll-top-btn'), condition, 'scale-1', 'scale-0');
    toggleClass(document.querySelector('#nav-menu'), condition, 'nav-shadow');
    toggleClass(document.querySelector('#nav-logo'), condition, 'img-size-scrolled');

    //

    let scrollPosition = document.documentElement.scrollTop || document.body.scrollTop;

    for (i in sections) {
        if (sections[i] <= scrollPosition) {
            document.querySelector('.active').setAttribute('class', '');
            document.querySelector('a[href*=' + i + ']').parentElement.setAttribute('class', 'active');
        }
    }
});

function refreshScrollspyItems() {
    sections = {};
    Array.prototype.forEach.call(document.querySelectorAll(".page-section"), function (e) {
        sections[e.id] = e.offsetTop - (document.querySelector('nav').offsetHeight - 50);
    });
}

function toggleClass(element, value, on, off) {
    if (value) {
        element.classList.add(on);
        if (off !== '' || off === null) element.classList.remove(off);
    } else {
        if (off !== '' || off === null) element.classList.add(off);
        element.classList.remove(on);
    }
}

function scrollTo(selector) {
    document.querySelector(selector).scrollIntoView({
        behavior: 'smooth'
    });
}

function scrollToPx(top, left) {
    window.scroll({
        top: top,
        left: left,
        behavior: 'smooth'
    });
}

function dismissMobileMenu(e) {
    document.querySelector('nav ul').style.setProperty('height', '0');
    isMobileMenuToggled = false;

    setMobileMenuBtnIcon();
}

function setMobileMenuBtnIcon() {
    document.querySelector('#mobile-menu-btn').innerHTML =
        isMobileMenuToggled ?
            `<svg viewBox="0 0 781 781" xmlns="http://www.w3.org/2000/svg" >
              <path d="M436,390.5L781,736L736,781L390.5,436L45,781L0,736L345,390.5L0,45L45,0L390.5,345L736,0L781,45Z"></path>
            </svg>`
            :
            `<svg viewBox="0 0 1024 576" xmlns="http://www.w3.org/2000/svg" >
              <path d="M1024,64L0,64L0,0L1024,0L1024,64ZM1024,576L0,576L0,512L1024,512L1024,576ZM1024,320L0,320L0,256L1024,256L1024,320Z"></path>
            </svg>`;
}

function correctMobileScroll() {
    if (isMobile && window.scrollY < 280) {
        scrollToPx(280, 0);
    }
}

window.addEventListener('DOMContentLoaded', (event) => {

    // Mobile detection and init tasks

    isMobile = window.innerWidth < 1080;

    correctMobileScroll();

    refreshScrollspyItems();

    // Mobile menu button click/tap

    document.querySelector('#mobile-menu-btn').addEventListener('click', () => {
        if (isMobile) {
            isMobileMenuToggled = !isMobileMenuToggled;
            document.querySelector('.hide-on-desktop + ul').style.setProperty('height', isMobileMenuToggled ? '12rem' : '0');

            setMobileMenuBtnIcon();
        }
    });

    // Mobile menu dismiss on link tap

    Array.prototype.forEach.call(document.querySelectorAll('nav ul li'), function (e) {
        e.addEventListener('click', dismissMobileMenu);
    });

    // 'Scroll to top' button click/tap

    document.getElementById('scroll-top-btn').addEventListener('click', () => {
        scrollToPx(isMobile ? 280 : 0, 0);
    });

    // Load last theme

    switchTheme(document.cookie.indexOf('dark') !== -1);

    // Theme switch logic

    document.getElementById('theme-switch-btn').addEventListener('click', () => {
        let isDark = document.cookie.indexOf('dark') !== -1;

        switchTheme(!isDark);

        document.cookie = `theme=${isDark ? 'light' : 'dark'}; expires=Thu, 31 Dec 2099 12:00:00 UTC;`;
    });
});

function switchTheme(isDark) {

    // Setting 'switch theme' button icon

    document.getElementById('theme-switch-btn').innerHTML =
        isDark ?
            '<svg height="100%" viewBox="0 0 960 960" width="100%" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink">\n' +
            '  <path d="M480,256C511,256 540.083,261.833 567.25,273.5C594.417,285.167 618.167,301.167 638.5,321.5C658.833,341.833 674.833,365.583 686.5,392.75C698.167,419.917 704,449 704,480C704,511 698.167,540.083 686.5,567.25C674.833,594.417 658.833,618.167 638.5,638.5C618.167,658.833 594.417,674.833 567.25,686.5C540.083,698.167 511,704 480,704C449,704 419.917,698.167 392.75,686.5C365.583,674.833 341.833,658.833 321.5,638.5C301.167,618.167 285.167,594.417 273.5,567.25C261.833,540.083 256,511 256,480C256,449 261.833,419.917 273.5,392.75C285.167,365.583 301.167,341.833 321.5,321.5C341.833,301.167 365.583,285.167 392.75,273.5C419.917,261.833 449,256 480,256ZM480,640C502,640 522.667,635.833 542,627.5C561.333,619.167 578.25,607.75 592.75,593.25C607.25,578.75 618.75,561.75 627.25,542.25C635.75,522.75 640,502 640,480C640,458 635.75,437.333 627.25,418C618.75,398.667 607.25,381.75 592.75,367.25C578.25,352.75 561.333,341.25 542,332.75C522.667,324.25 502,320 480,320C458,320 437.25,324.25 417.75,332.75C398.25,341.25 381.25,352.75 366.75,367.25C352.25,381.75 340.833,398.667 332.5,418C324.167,437.333 320,458 320,480C320,502 324.167,522.75 332.5,542.25C340.833,561.75 352.25,578.75 366.75,593.25C381.25,607.75 398.25,619.167 417.75,627.5C437.25,635.833 458,640 480,640ZM512,192L448,192L448,0L512,0ZM448,768L512,768L512,960L448,960ZM960,448L960,512L768,512L768,448ZM192,512L0,512L0,448L192,448ZM253.5,299L118,163L163,118L299,253.5ZM706.5,661L842,797L797,842L661,706.5ZM706.5,299L661,253.5L797,118L842,163ZM253.5,661L299,706.5L163,842L118,797Z">\n' +
            '  </path>\n' +
            '</svg>'
            :
            '<svg height="100%" viewBox="0 0 958 1020" width="100%" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink">\n' +
            '  <path d="M61.5,764C108.5,764 153.833,757.917 197.5,745.75C241.167,733.583 281.917,716.333 319.75,694C357.583,671.667 392.083,644.917 423.25,613.75C454.417,582.583 481.167,548.083 503.5,510.25C525.833,472.417 543.083,431.667 555.25,388C567.417,344.333 573.5,299 573.5,252C573.5,207.667 567.833,164.167 556.5,121.5C545.167,78.8334 528.667,38.3334 507,0C549.667,5.33337 590.5,15.5 629.5,30.5C668.5,45.5 704.667,64.6667 738,88C771.333,111.333 801.5,138.25 828.5,168.75C855.5,199.25 878.583,232.417 897.75,268.25C916.917,304.083 931.667,342.25 942,382.75C952.333,423.25 957.5,465 957.5,508C957.5,555 951.417,600.333 939.25,644C927.083,687.667 909.833,728.417 887.5,766.25C865.167,804.083 838.417,838.583 807.25,869.75C776.083,900.917 741.583,927.667 703.75,950C665.917,972.333 625.167,989.583 581.5,1001.75C537.833,1013.92 492.5,1020 445.5,1020C399.5,1020 354.667,1014 311,1002C267.333,990 226.25,972.75 187.75,950.25C149.25,927.75 114,900.417 82,868.25C50,836.083 22.6667,800 0,760C10.3333,761.333 20.5833,762.333 30.75,763C40.9167,763.667 51.1667,764 61.5,764ZM445.5,956C486.833,956 526.583,950.667 564.75,940C602.917,929.333 638.583,914.25 671.75,894.75C704.917,875.25 735.083,851.917 762.25,824.75C789.417,797.583 812.75,767.417 832.25,734.25C851.75,701.083 866.833,665.417 877.5,627.25C888.167,589.083 893.5,549.333 893.5,508C893.5,462.667 886.833,418.75 873.5,376.25C860.167,333.75 841.417,294.417 817.25,258.25C793.083,222.083 763.833,189.833 729.5,161.5C695.167,133.167 657,110.5 615,93.5C630,145.167 637.5,198 637.5,252C637.5,300.667 631.667,347.75 620,393.25C608.333,438.75 591.667,481.75 570,522.25C548.333,562.75 522.167,600.25 491.5,634.75C460.833,669.25 426.667,699.583 389,725.75C351.333,751.917 310.583,773.417 266.75,790.25C222.917,807.083 176.833,818.333 128.5,824C171.167,866.667 219.583,899.333 273.75,922C327.917,944.667 385.167,956 445.5,956Z">\n' +
            '  </path>\n' +
            '</svg>';


    // Setting 'switch theme' button tooltip

    document.querySelector('#theme-switch-btn').setAttribute('tooltipText', isDark ? 'Light mode' : 'Dark mode');

    // Setting theme colors

    let root = document.documentElement;

    if (isDark) {
        root.style.setProperty('--primary-color', 'rgb(173, 20, 87)');
        root.style.setProperty('--primary-color-values', '173, 20, 87');
        root.style.setProperty('--primary-foreground', '#fff');
        root.style.setProperty('--body-text', '#eee');
        root.style.setProperty('--body-text-light', 'rgba(238, 238, 238, 0.5)');
        root.style.setProperty('--body-text-values', '238, 238, 238');
        root.style.setProperty('--background', '#141414');
        root.style.setProperty('--code-bg', '#242424');
        root.style.setProperty('--code-fg', '#eee');
        root.style.setProperty('--shadow-color', 'rgba(80, 80, 80, 0.3)');
        root.style.setProperty('--shadow-color-dark', 'rgba(80, 80, 80, 0.5)');
    } else {
        root.style.setProperty('--primary-color', 'rgb(173, 20, 87)');
        root.style.setProperty('--primary-color-values', '173, 20, 87');
        root.style.setProperty('--primary-foreground', '#fff');
        root.style.setProperty('--body-text', '#222');
        root.style.setProperty('--body-text-values', '34, 34, 34');
        root.style.setProperty('--body-text-light', 'rgba(34, 34, 34, 0.5)');
        root.style.setProperty('--background', '#fff');
        root.style.setProperty('--code-bg', '#f1f1f1');
        root.style.setProperty('--code-fg', '#222');
        root.style.setProperty('--shadow-color', 'rgba(0, 0, 0, 0.2)');
        root.style.setProperty('--shadow-color-dark', 'rgba(0, 0, 0, 0.4)');
    }
}
