// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{Engine, Error, Signal};
fn order(engine: &Engine) -> Vec<u64> {
    engine
        .draw_order()
        .unwrap()
        .into_iter()
        .map(|h| h.id())
        .collect()
}
#[test]
fn native_z_order_and_lifetimes() {
    let mut engine = Engine::new().unwrap();
    let mut handles = Vec::new();
    for _ in 0..3 {
        handles.push(
            engine
                .spawn(Default::default(), |p| async move {
                    loop {
                        p.frame().await;
                    }
                })
                .unwrap(),
        );
    }
    assert_eq!(handles[0].z().unwrap(), 0.0);
    handles[0].set_z(10.0).unwrap();
    assert_eq!(order(&engine), vec![2, 3, 1]);
    handles[0].set_priority(-999).unwrap();
    assert_eq!(order(&engine), vec![2, 3, 1]);
    for z in [f64::NAN, f64::INFINITY, f64::NEG_INFINITY] {
        assert_eq!(handles[0].set_z(z), Err(Error::InvalidArgument));
    }
    assert_eq!(handles[0].z().unwrap(), 10.0);
    handles[0].set_z(f64::MAX).unwrap();
    handles[0].set_z(-1.0).unwrap();
    assert_eq!(order(&engine), vec![1, 2, 3]);
    engine.signal(&handles[1], Signal::Sleep).unwrap();
    engine.signal(&handles[2], Signal::Freeze).unwrap();
    assert_eq!(order(&engine), vec![1, 3]);
    engine.signal(&handles[1], Signal::Wake).unwrap();
    assert_eq!(order(&engine), vec![1, 2, 3]);
    engine.tick().unwrap();
    handles[0].kill().unwrap();
    assert_eq!(order(&engine), vec![2, 3]);
    assert_eq!(handles[0].z(), Err(Error::NotFound));
    assert_eq!(handles[0].set_z(1.0), Err(Error::NotFound));
    let copied = engine.draw_order().unwrap();
    handles[1].set_z(100.0).unwrap();
    assert_eq!(
        copied.iter().map(|h| h.id()).collect::<Vec<_>>(),
        vec![2, 3]
    );
    drop(engine);
    assert_eq!(handles[1].z(), Err(Error::Closed));
    assert_eq!(handles[1].set_z(0.0), Err(Error::Closed));
}

#[test]
fn native_order_matches_many_duplicate_and_fractional_depths() {
    let engine = Engine::new().unwrap();
    let mut expected = Vec::new();
    for index in 0..257 {
        let handle = engine
            .spawn(Default::default(), |p| async move {
                loop {
                    p.frame().await;
                }
            })
            .unwrap();
        let z = ((index * 37) % 23) as f64 / 2.0 - 5.0;
        handle.set_z(z).unwrap();
        expected.push((z, handle.id()));
    }
    expected.sort_by(|a, b| a.0.partial_cmp(&b.0).unwrap().then(a.1.cmp(&b.1)));
    assert_eq!(
        order(&engine),
        expected.iter().map(|p| p.1).collect::<Vec<_>>()
    );
    assert_eq!(
        order(&engine),
        expected.iter().map(|p| p.1).collect::<Vec<_>>()
    );
}
