create database if not exists musicLibrary;

use musicLibrary;

drop table if exists songs;
drop table if exists albums;
drop table if exists artists;

create table if not exists artists(
    artistId int unsigned not null unique auto_increment,
    artistName varchar(256) not null,
    createdDate timestamp not null default now(),
    modifiedDate timestamp on update now(),
    primary key(artistId)
);

create table if not exists albums(
    albumId int unsigned not null unique auto_increment,
    artistId int unsigned not null,
    albumYear bigint not null,
    albumName varchar(256) not null,
    createdDate timestamp not null default now(),
    modifiedDate timestamp on update now(),
    primary key(albumId),
    foreign key(artistId) references artists(artistId)
);

create table if not exists songs(
    songId int unsigned not null unique auto_increment,
    artistId int unsigned not null,
    albumId int unsigned not null,
     position int unsigned not null,
    songTitle varchar(256) not null,
    plays int unsigned not null default 0,
    createdDate timestamp not null default now(),
    modifiedDate timestamp on update now(),
    foreign key(albumId) references albums(albumId),
    foreign key(artistId) references artists(artistId),
    constraint uc_position unique(albumId, position)
);
